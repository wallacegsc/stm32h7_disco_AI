#include "serial.hpp"
#include <cstdio>
#include <cstdarg>


void LoggerSerial::log_with_prefix(const char* prefix, const char* fmt, va_list args)
{
    int offset = snprintf(buffer_, sizeof(buffer_), "%s", prefix);
    if (offset < 0) return;

    int n = vsnprintf(buffer_ + offset, sizeof(buffer_) - offset, fmt, args);
    if (n < 0) return;

    int total = offset + n;
    if (total > (int)sizeof(buffer_) - 1) total = sizeof(buffer_) - 1;  // clamp se truncou

    out_.write_bytes((uint8_t*)buffer_, total);
    out_.write_bytes((uint8_t*)"\r\n", 2);
}

void LoggerSerial::log_raw(const char* fmt, ...)
{
    if(level_ >= Level::None) return;

    va_list args;
    va_start(args, fmt);

    int len = vsnprintf(buffer_, sizeof(buffer_), fmt, args);
    if (len > 0) {
        if (len > (int)sizeof(buffer_) - 1) len = sizeof(buffer_) - 1;  // truncou? ajusta
        out_.write_bytes((uint8_t*)buffer_, len);
    }

    va_end(args);
}
void LoggerSerial::log_info(const char* fmt, ...)
{
    if(level_ > Level::Info) return;
    va_list args;
    va_start(args, fmt);
    this->log_with_prefix("I: ",fmt, args);
    va_end(args);
}
void LoggerSerial::log_error(const char* fmt, ...)
{
    if(level_ > Level::Error) return;
    va_list args;
    va_start(args, fmt);
    this->log_with_prefix("E: ",fmt, args);
    va_end(args);
}
void LoggerSerial::log_debug(const char* fmt, ...)
{
    if(level_ > Level::Debug) return;
    va_list args;
    va_start(args, fmt);
    this->log_with_prefix("D: ",fmt, args);
    va_end(args);
}

//ClaudeAI
void MenuSerial::autocomplete()
{
    std::string_view prefix(read_buf_.data(), read_buf_pos_);
    if (prefix.empty())
        return;

    const option* matches[MAX_OPTIONS];
    std::size_t   n_matches = 0;

    for (std::size_t i = 0; i < ops_num_; ++i)
    {
        std::string_view cmd = ops_[i].cmd;
        if (cmd.size() >= prefix.size() &&
            cmd.substr(0, prefix.size()) == prefix)
        {
            matches[n_matches++] = &ops_[i];
        }
    }

    if (n_matches == 0)
        return;

    std::string_view common = matches[0]->cmd;
    for (std::size_t i = 1; i < n_matches; ++i)
    {
        std::string_view cmd = matches[i]->cmd;
        std::size_t j = 0;
        std::size_t lim = (common.size() < cmd.size()) ? common.size() : cmd.size();
        while (j < lim && common[j] == cmd[j])
            ++j;
        common = common.substr(0, j);
    }

    std::string_view to_add = common.substr(read_buf_pos_);

    if (!to_add.empty())
    {
        for (char ch : to_add)
        {
            if (read_buf_pos_ >= READ_BUF_SIZE - 1) break;
            read_buf_[read_buf_pos_++] = ch;
        }
        write(to_add);
    }
    else if (n_matches > 1)
    {
        write("\r\n");
        for (std::size_t i = 0; i < n_matches; ++i)
        {
            write(matches[i]->cmd);
            write("  ");
        }
        write("\r\nCmd?: ");
        write(std::string_view(read_buf_.data(), read_buf_pos_));
    }
}


// Stop Waiting Ready Full
MenuSerial::MenuState MenuSerial::consume_data()
{
    uint8_t b = 0;
    while(io_serial_.read_bytes(&b, 1) != 0){
        char c = static_cast<char>(b);

        if (c == '\t') { autocomplete(); return MenuState::Waiting; }

        if(c == '\b' || c == 0x7F) // backspace or delete
        {
            if(read_buf_pos_ > 0) {
                read_buf_pos_--;
                write("\b \b");
            }
            return MenuState::Waiting;
        }
    
        if(c == '\r' || c == '\n') // end of line
        {
            if(read_buf_pos_ == 0)
            {
                write("\r\nCmd?:");
                return MenuState::Waiting; // Ignores empty line
            }
            read_buf_[read_buf_pos_] = '\0';

            ready_read_buf_pos_ = read_buf_pos_; 
            read_buf_pos_ = 0;
            return MenuState::Ready;
        }

        if(read_buf_pos_ >= READ_BUF_SIZE - 1)
            return MenuState::Full; // (Reserved 1 byte for '\0')

        read_buf_[read_buf_pos_++] = c;
        write(std::string_view(&c,1));

    }
    return MenuState::Waiting;
}

int MenuSerial::instance_option(const option& op)
{
    if(ops_num_ >= MAX_OPTIONS) return -1;

    if(op.f == nullptr)
        return -1;

    ops_[ops_num_++] = op;
    return 0;
}

void MenuSerial::write(std::string_view s)
{
    io_serial_.write_bytes(
        reinterpret_cast<const uint8_t*>(s.data()),
        static_cast<int>(s.size())
    );
}

void MenuSerial::writef(const char* fmt, ...)
{
    char buf[64];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (n < 0) return;                              // erro de formatação
    if (n > static_cast<int>(sizeof(buf)) - 1)      // truncou? limita ao buffer
        n = static_cast<int>(sizeof(buf)) - 1;

    io_serial_.write_bytes(reinterpret_cast<const uint8_t*>(buf), n);
}

//Returns the op index or
MenuSerial::OptionState MenuSerial::processing(std::string_view line)
{
    write("\r\n\n>> Menu <<\r\n");
    write("Input: ");
    writef("%.*s\r\n", static_cast<int>(line.size()), line.data());

    std::size_t sp = line.find(' ');
    std::string_view cmd  = line.substr(0, sp);
    std::string_view args = (sp == std::string_view::npos)
                            ? std::string_view{}     
                            : line.substr(sp + 1);   

    writef("Cmd: %.*s | ", static_cast<int>(cmd.size()), cmd.data());
    if (args.empty())
        write("Args: Empty\r\n");
    else
        writef("Args: %.*s\r\n", static_cast<int>(args.size()), args.data());

    write("\r\n");
    bool find = false;
    if(cmd == "help")
    {
        write("\r\ncmd list:\r\n");
        write("--------------------------------------------------\r\n");
        for (std::size_t i = 0; i < ops_num_; ++i)
        {
            constexpr std::size_t COL = 24;  
            write("  ");
            write(ops_[i].cmd);
            std::size_t pad = (ops_[i].cmd.size() < COL)
                            ? COL - ops_[i].cmd.size() : 1;
            for (std::size_t p = 0; p < pad; ++p)
                write(" ");
            write(ops_[i].help);
            write("\r\n");
        }
        write("--------------------------------------------------\r\n");
        find = true;
    }
    else if(cmd == "clear"){
        write("\033[2J\033[H");
        find = true;
    }
    else
    {
        for (std::size_t i = 0; i < ops_num_; ++i)
        {
            if(cmd == ops_[i].cmd)
            {
                find = true;
                last_op_executed_ = i;
                return ops_[i].f(args, MenuState::Ready);
            }
        }
    }

    if(!find)
        write("\n\rNot found (type: help)\r\n");
    write("\r\nCmd?:");

    return OptionState::Nothing;
}

void MenuSerial::update_menu_state(OptionState op_state)
{
    switch (op_state)
    {
    case OptionState::Finish:
    case OptionState::Nothing:
        last_op_executed_ = NO_EXECUTION;
        menu_state_ = MenuState::Waiting;
        break;
    case OptionState::Running:
        menu_state_ = MenuState::Loop;
        break;
    }
}

bool MenuSerial::key_interrupt()
{
    uint8_t b = 0;
    while (io_serial_.read_bytes(&b, 1) != 0)
    {
        if (b == 0x03)   // Ctrl+C
        {
            write("\r\n^C\r\n");
            return true;
        }
    }
    return false;
}

void MenuSerial::run()
{
    OptionState op_state = OptionState::Nothing;

    switch (menu_state_)
    {
    case MenuState::Waiting:
        if (io_serial_.available())
            menu_state_ = consume_data();   
        return;

    case MenuState::Ready:
        op_state = processing(std::string_view(read_buf_.data(), ready_read_buf_pos_));
        if(last_op_executed_ != NO_EXECUTION)
        {
            if(op_state != OptionState::Finish && op_state != OptionState::Running )
            {
                writef("Error(%.*s): Option must be Finish or Running\r\n",
                    static_cast<int>(ops_[last_op_executed_].cmd.size()),
                    ops_[last_op_executed_].cmd.data());
                op_state = OptionState::Finish;
            }
        }
        break;

    case MenuState::Loop:
        if (key_interrupt())
        {
            menu_state_ = MenuState::Stop;
            break;                 
        }
        
        op_state = ops_[last_op_executed_].f(std::string_view{}, MenuState::Loop);
        break;

    case MenuState::Stop:
        op_state = ops_[last_op_executed_].f(std::string_view{}, MenuState::Stop);
        if(op_state != OptionState::Finish)
        {
            writef("Error(%.*s): Option must finish with Menu Stop, forcing...\r\n",
                static_cast<int>(ops_[last_op_executed_].cmd.size()),
                ops_[last_op_executed_].cmd.data());

            op_state = OptionState::Finish;
        }
        break;

    default:
        break;
    }

    update_menu_state(op_state);
}