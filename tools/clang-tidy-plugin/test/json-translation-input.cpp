// RUN: %check_clang_tidy %s cata-json-translation-input %t -- --load=%cata_plugin -- -I %test_include

#include "mock-translation.h"

// dummy json interface
class JsonArray
{
    public:
        std::string next_string();
};

class JsonObject
{
    public:
        std::string get_string( const std::string & );
        JsonArray get_array( const std::string & );
        template<class T>
        bool read( const std::string &name, T &t, bool throw_on_error = true );
};

class JsonIn
{
    public:
        JsonObject get_object();
};

class foo
{
    public:
        std::string name;
        std::string double_name;
        std::string nickname;
        translation msg;
        translation more_msg;
        translation moar_msg;
        translation endless_msg;
        translation desc;
        std::string whatever;
};

// <translation function>( ... <json input object>.<method>(...) ... )
static void deserialize( foo &bar, JsonIn &jin )
{
    JsonObject jo = jin.get_object();
    bar.name = _( jo.get_string( "name" ) );
    // CHECK-MESSAGES: [[@LINE-1]]:16: warning: immediately translating a value read from json causes translation updating issues.  Consider reading into a translation object instead.
    // CHECK-MESSAGES: [[@LINE-2]]:19: note: value read from json

    JsonArray ja = jo.get_array( "double_name" );
    bar.double_name = std::string( _( ja.next_string() ) ) + pgettext( "blah",
                      // CHECK-MESSAGES: [[@LINE-1]]:36: warning: immediately translating a value read from json causes translation updating issues.  Consider reading into a translation object instead.
                      // CHECK-MESSAGES: [[@LINE-2]]:39: note: value read from json
                      // CHECK-MESSAGES: [[@LINE-3]]:62: warning: immediately translating a value read from json causes translation updating issues.  Consider reading into a translation object instead.
                      std::string( ja.next_string() ) );
    // CHECK-MESSAGES: [[@LINE-1]]:36: note: value read from json

    // ok, not reading from json
    bar.nickname = _( "blah" );

    bar.msg = to_translation( jo.get_string( "message" ) );
    // CHECK-MESSAGES: [[@LINE-1]]:15: warning: read translation directly instead of constructing it from json strings to ensure consistent format in json.
    // CHECK-MESSAGES: [[@LINE-2]]:31: note: string read from json

    bar.more_msg = pl_translation( jo.get_string( "message" ), "foo" );
    // CHECK-MESSAGES: [[@LINE-1]]:20: warning: read translation directly instead of constructing it from json strings to ensure consistent format in json.
    // CHECK-MESSAGES: [[@LINE-2]]:36: note: string read from json

    bar.moar_msg = translation::to_translation( jo.get_string( "more_message" ) );
    // CHECK-MESSAGES: [[@LINE-1]]:20: warning: read translation directly instead of constructing it from json strings to ensure consistent format in json.
    // CHECK-MESSAGES: [[@LINE-2]]:49: note: string read from json

    // ok, reading translation directly
    jo.read( "endless_msg", bar.endless_msg );

    // ok, using no_translation to avoid re-translating generated string
    bar.desc = no_translation( jo.get_string( "generated_desc" ) );

    std::string duh;
    // ok, not reading from json
    bar.whatever = _( duh.c_str() );
}
