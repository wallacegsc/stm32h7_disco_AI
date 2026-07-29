#pragma once

#include <cstdint>
#include <array>
#include <string_view>
#include <cstdarg>


/*
    
    read:
        Must Implement the reading in the background and exports data through available and read_bytes
*/
class Serial{
public:
        virtual int write_bytes(const uint8_t* buffer, int len) = 0;
        virtual ~Serial() = default;

        // No-bloking return readed bytes or 0 if no read
        virtual int read_bytes(uint8_t* buffer, int len) = 0;
        virtual bool available() const = 0; //Bytes ready?
private:
};  

class MenuSerial{
public:

    enum class MenuState{Waiting, Loop, Ready, Stop, Full};
    enum class OptionState{
        Finish, Running, // Return from option execution
        Nothing //Used by the Menu
    };

    using Handler = OptionState(*)(std::string_view args, MenuState menu_state);

    struct option{
        std::string_view cmd;
        std::string_view help;
        Handler f;
    };

    MenuSerial(Serial& s) : io_serial_{s} {};
    // ~MenuSerial(){};

    int instance_option(const option& op);
    void run();

private:
    Serial& io_serial_;

    static constexpr std::size_t READ_BUF_SIZE = 32;
    std::array<char, READ_BUF_SIZE> read_buf_{};
    std::size_t read_buf_pos_ = 0;
    
    static constexpr std::size_t MAX_OPTIONS = 10;
    std::array<option, MAX_OPTIONS> ops_{};
    std::size_t ops_num_ = 0;

    constexpr static std::size_t NO_EXECUTION = -1;

    MenuState menu_state_ = MenuState::Waiting;
    std::size_t ready_read_buf_pos_ = 0;
    std::size_t last_op_executed_ = NO_EXECUTION; // Index of read_buf_


    void write(std::string_view s);
    void writef(const char* fmt, ...);
    void autocomplete();
    void update_menu_state(OptionState op_state);
    bool key_interrupt();
    MenuState consume_data();
    MenuSerial::OptionState processing(std::string_view line);
};


class LoggerSerial{
    Serial& out_;
    static constexpr size_t BUF_SIZE = 128;
    char buffer_[BUF_SIZE];

public:
    enum class Level {Debug = 0, Info = 1, Error = 2, None = 3};

    explicit LoggerSerial(Serial& s) : out_{s}{}

    void set_level(Level l) { level_ = l; }

    void log_raw(const char* fmt, ...);
    void log_info(const char* fmt, ...);
    void log_error(const char* fmt, ...);
    void log_debug(const char* fmt, ...);

private:
    Level level_ = Level::Debug;
    
    void log_with_prefix(const char* prefix, const char* fmt, va_list args);
};