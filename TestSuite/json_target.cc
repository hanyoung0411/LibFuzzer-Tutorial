#include "json.hpp"
using json = nlohmann::json;
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    try{
        // step 1: parse input
        json j1 = json::parse(data, data + size);
        try{
            // step 2: round trip
            // first serialization
            std::string s1 = j1.dump();
            // parse serialized string again
            json j2 = json::parse(s1);
            std::string s2 = j2.dump();
            // serializations must match
            assert(s1 == s2);
        }
        catch (const std::invalid_argument&){
            // parsing a JSON serialization must not fail
            assert(false);
        }
    }
    catch (const std::invalid_argument&){
        // parse errors are ok, because input may be random bytes
    }
    return 0;
}
