#ifdef _MSC_VER
#pragma warning(disable : 4786)  // identifier too long
#endif


#ifndef __FilePath_H
#define __FilePath_H

#include <sstream>
#include <list>
#include <string>
#ifndef _WIN32_WCE
#include <locale>
#endif
#include <cstdlib>

// path_delimiters<charT>
template<typename charT>
class path_delimiters
{
public:
    static bool is_path_delim(charT c);
    static bool is_drive_delim(charT c);
    static bool is_scheme_delim(charT c);
    static bool is_port_delim(charT c);
    static bool is_query_delim(charT c);
    static bool is_extension_delim(charT c);
    
    static charT path_delim();
    static charT drive_delim();
    static charT scheme_delim();
    static charT port_delim();
    static charT query_delim();
    static charT extension_delim();

    static const charT* unknown_scheme();
};

// }<char>
template<>
class path_delimiters<char>
{
public:
    static bool is_path_delim(char c)      { return (c == '/') || (c == '\\'); }
    static bool is_drive_delim(char c)     { return (c == ':'); }
    static bool is_scheme_delim(char c)    { return (c == ':'); }
    static bool is_port_delim(char c)      { return (c == ':'); }
    static bool is_query_delim(char c)     { return (c == '?'); }
    static bool is_extension_delim(char c) { return (c == '.'); }

    static char path_delim()      { return '/'; }
    static char drive_delim()     { return ':'; }
    static char scheme_delim()    { return ':'; }
    static char port_delim()      { return ':'; }
    static char query_delim()     { return '?'; }
    static char extension_delim() { return '.'; }

    static const char* unknown_scheme() { return "unknown"; }
};

// path_delimiters<wchar_t>
template<>
class path_delimiters<wchar_t>
{
public:
    static bool is_path_delim(wchar_t c)      { return (c == L'/') || (c == L'\\'); }
    static bool is_drive_delim(wchar_t c)     { return (c == L':'); }
    static bool is_scheme_delim(wchar_t c)    { return (c == L':'); }
    static bool is_port_delim(wchar_t c)      { return (c == L':'); }
    static bool is_query_delim(wchar_t c)     { return (c == L'?'); }
    static bool is_extension_delim(wchar_t c) { return (c == L'.'); }

    static wchar_t path_delim()      { return L'/'; }
    static wchar_t drive_delim()     { return L':'; }
    static wchar_t scheme_delim()    { return L':'; }
    static wchar_t port_delim()      { return L':'; }
    static wchar_t query_delim()     { return L'?'; }
    static wchar_t extension_delim() { return L'.'; }

    static const wchar_t* unknown_scheme() { return L"unknown"; }
};



// basic_filepath<charT, delimT>
template<typename charT, typename delimT = path_delimiters<charT> >
class basic_filepath
{
public:
    typedef std::basic_string<charT> string;


    // constructors

    basic_filepath() {
        // empty path
        parse(string().c_str());
    }


    basic_filepath(const charT* path) {
        parse(path);
    }

    // operator overloads
    basic_filepath<charT, delimT>&
    operator=(const basic_filepath<charT, delimT>& rhs) {
        if (this != &rhs) {
            parse(rhs.str().c_str());
        }
    }


    // stringify
    string str() const {
		std::basic_ostringstream<charT> s;
        
        // add URL elements
        if (is_url()) {
            s << m_scheme;
            s << delimT::scheme_delim();
            s << delimT::path_delim();
            s << delimT::path_delim();
            s << m_server;
            if (m_port != 0) {
                s << delimT::port_delim();
                s << m_port;
            }
        }

        // add Windows elements
        if (is_windows()) {
            s << m_drive;
            s << delimT::drive_delim();
        }

        // add leading path delimiter if absolute
        if (m_is_absolute) {
            s << delimT::path_delim();
        }

        // add directories
		for (std::list<std::string>::const_iterator i = m_path_segments.begin();
             i != m_path_segments.end();
             i++) {
            s << *i << delimT::path_delim();
        }

        // add file and extension
        if (m_file.length() > 0) {
            s << m_file;
            if (m_extension.length() > 0) {
                s << delimT::extension_delim() << m_extension;
            }
        }

        // final URL elements
        if (is_url() && m_query.length() > 0) {
            s << delimT::query_delim() << m_query;
        }

        return s.str();
    }


    // Windows/DOS
    
    bool is_windows() const {
        return (m_drive != charT());
    }


    void set_drive(charT drive) const {
        m_drive = drive;
    }


    charT get_drive() const {
        return m_drive;
    }


    // URL

    bool is_url() const {
        return (m_scheme.length() != 0);
    }


    void set_scheme(const string& scheme) {
        m_scheme = scheme;
    }


    string get_scheme() const {
        return m_scheme;
    }

    void set_server(const string& server) {
        if (is_url() == false) {
            m_scheme = delimT::unknown_scheme();
        }
        m_server = server;
    }

    string get_server() const {
        return m_server;
    }

    void set_port(int port) {
        if (is_url() == false) {
            m_scheme = delimT::unknown_scheme();
        }
        m_port = port;
    }

    int get_port() const {
        return m_port;
    }

    void set_query(const string& query) {
        if (is_url() == false) {
            m_scheme = delimT::unknown_scheme();
        }
        m_query = query;
    }

    string get_query() const {
        return m_query;
    }


    // general        

    bool is_absolute() const {
        return m_is_absolute;
    }


    string get_path() const {
        string r;
        if (m_is_absolute) {
            r = delimT::path_delim();
        }

		for (std::list<std::string>::const_iterator i = m_path_segments.begin();
             i != m_path_segments.end();
             i++) {
            r += *i;
            r += delimT::path_delim();
        }
        return r;
    }

	string get_path_except_extension() const
	{
		return get_path_except_filename() + get_file();
	}

	string get_path_except_filename() const
	{
		charT  drv = get_drive();
		if('\0' == drv)
			return get_path(); 
		return string() + drv + ":" + get_path();
	}

    bool has_file() const {
        return (m_file.length() != 0);
    }


    void set_file(const string& file) {
        m_file = file;
    }


    string get_file() const {
        return m_file;
    }


    void set_extension(const string& extension) {
        m_extension = extension;
    }

    
    string get_extension() {
        return m_extension;
    }


private:
    void parse(const charT* s) {
        // initialize
        m_is_absolute = false;
        m_drive = charT();
        m_scheme = string();
        m_server = string();
        m_port = 0;
        m_query = string();
        m_path_segments.clear();
        m_file = string();
        m_extension = string();

        // check for Windows path first
#ifndef _WIN32_WCE
        //if (std::isalpha(s[0], std::locale::empty()) &&
		if (std::isalpha(s[0], std::locale()) &&
                delimT::is_drive_delim(s[1]) &&
                delimT::is_path_delim(s[2])) 
#else
		if (isalpha(s[0]) &&
				delimT::is_drive_delim(s[1]) &&
				delimT::is_path_delim(s[2]))
#endif
		{
            m_drive = s[0];
            m_is_absolute = true;
            s += 3;
        }

        const charT* p = s;

        if (!is_windows()) {
            // check for URL
            while (*p) {
                if (delimT::is_scheme_delim(*p)) {
                    m_scheme = string(s, p - s);
                    s = p + 1;

                    // possibly skip the next two path delimiters
                    if (delimT::is_path_delim(*s)) {
                        s++;
                    }
                    if (delimT::is_path_delim(*s)) {
                        s++;
                    }
                    break;
                }
                p++;
            }

            // if we found a URL, parse server/port
            if (m_scheme.length() > 0) {
            
                // server
                while (*s) {
                    if (delimT::is_port_delim(*s) ||
                        delimT::is_path_delim(*s)) {
                        break;
                    }
                    m_server += *s;
                    s++;
                }

                // port
                if (delimT::is_port_delim(*s)) {
                    s++;
                    string port;
                    while (*s && !delimT::is_path_delim(*s)) {
                        port += *s;
                        s++;
                    }

					std::basic_istringstream<charT> is(port);
                    is >> m_port;
                }
            }
        }

        // parse path list
        string c;
        while (*s) {
            // if it's a path delimiter, add it to the path list
            if (delimT::is_path_delim(*s)) {
                if (c.length() > 0) {
                    m_path_segments.push_back(c);
                } else {
                    // if first segment is empty
                    // (first path character is /),
                    // path is absolute
                    if (m_path_segments.size() == 0) {
                        m_is_absolute = true;
                    }
                }
                c = string();
                s++;
                continue;
            }

            c += *s;
            s++;
        }

        // parse filename in c
        const charT* q = c.c_str();
        p = q;
        while (*p) {
            if (delimT::is_query_delim(*p)) {
                // grab the query string
                m_query = p + 1;
                break;
            }
            p++;
        }

        // get a file+extension string
        string file(q, p - q);
        int pos = -1;
        for (int i = file.length() - 1; i >= 0; i--) {
            if (delimT::is_extension_delim(file[i])) {
                pos = i;
                break;
            }
        }

        if (pos == -1) {
            m_file = file;
        } else {
            m_file = file.substr(0, pos);
            m_extension = file.substr(pos + 1, file.length() - pos - 1);
        }
    }


private:
    bool m_is_absolute;

    // windows
    charT m_drive;

    // URL
    string m_scheme;
    string m_server;
    int    m_port;
    string m_query;

    // general
    std::list<string> m_path_segments;
    string m_file;
    string m_extension;        
};


typedef basic_filepath<char>    filepath;
typedef basic_filepath<wchar_t> wfilepath;


#endif // __FilePath_H
