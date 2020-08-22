#pragma once

class ILog
{
public:
    template<typename... Args>
    void Debug(const char* fmt, Args... args)
    {
        LogMessage(_DMESSAGE, nullptr, fmt, args...);
    }

    template<typename... Args>
    void Message(const char* fmt, Args... args)
    {
        LogMessage(_MESSAGE, nullptr, fmt, args...);
    }

    template<typename... Args>
    void Warning(const char* fmt, Args... args)
    {
        LogMessage(_WARNING, "WARNING", fmt, args...);
    }

    template<typename... Args>
    void Error(const char* fmt, Args... args)
    {
        LogMessage(_ERROR, "ERROR", fmt, args...);
    }

    template<typename... Args>
    void FatalError(const char* fmt, Args... args)
    {
        LogMessage(_FATALERROR, "FATAL", fmt, args...);
    }

    static IDebugLog::LogLevel TranslateLogLevel(const std::string& level);

    FN_NAMEPROC("ILog")
private:
    typedef std::unordered_map<std::string, IDebugLog::LogLevel> logLevelMap_t;

    template<typename F, typename... Args>
    void LogMessage(F a_proc, const char* a_pfix, const char* a_fmt, Args... a_args)
    {
        m_logMutex.Enter();

        std::ostringstream _fmt;
        if (a_pfix != nullptr)
            _fmt << "<" << a_pfix << "> ";
        _fmt << "[" << ModuleName() << "] " << a_fmt;

        auto str = _fmt.str();
        a_proc(str.c_str(), a_args...);

        m_logMutex.Leave();

        std::unique_ptr<char[]> b(new char[8192]);
        _snprintf_s(b.get(), 8192, _TRUNCATE, str.c_str(), a_args...);
        CBP::IEvents::TriggerEvent(CBP::Event::OnLogMessage, b.get());
    }

    static logLevelMap_t LogLevelMap;
    static ICriticalSection m_logMutex;
};
