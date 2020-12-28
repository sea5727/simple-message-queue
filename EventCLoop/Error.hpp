#pragma once



namespace EventCLoop
{
    class Error {
        std::string msg;
        bool flag = false;
    public:
        Error() = default;
        Error(const std::string & what)
            : msg{what}
            , flag{true} {}

        operator bool() const{
            return flag;
        }
        const char * what() const {
            return msg.c_str();
        }
    };
}