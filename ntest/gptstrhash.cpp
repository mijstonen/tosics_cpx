#!/usr/local/bin/cpx
namespace gpt{
constexpr uint64_t string_to_uint64(std::string_view str) {
    uint64_t result = 0;
    for (size_t i = 0; i < str.size(); ++i) {
        result = (result << 8) | static_cast<unsigned char>(str[i]);
    }
    return result;
}

constexpr uint64_t hash(const char* str) {
    return string_to_uint64(str);
}
}



#!
    char c=48;
    cout<<"case '\\0': return 0;\n";
    for(unsigned u=0;;)
    {
        if (isalnum(c)) {
            //INFO(++u,static_cast<int>(c), c);
            cout<<"case '"<<c<<"': return "<<++u<<";\n";
        }

        if (++c==127)break;
    }
    cout<<"default: return 63"<<endl;


    // 64 bits -> 10*[0..63] \0 1..9 a..z, A..Z _ + 1*[0..15]

    constexpr uint64_t case1 = gpt::hash("abcdefg");
    constexpr uint64_t case2 = gpt::hash("1234567");
    constexpr uint64_t case3 = gpt::hash("xyz1234");

    uint64_t input = case1;

    switch (input) {
        case case1:
            std::cout << "Matched case 1: \"abcdefg\"" << std::endl;
            break;
        case case2:
            std::cout << "Matched case 2: \"1234567\"" << std::endl;
            break;
        case case3:
            std::cout << "Matched case 3: \"xyz1234\"" << std::endl;
            break;
        default:
            std::cout << "No match found." << std::endl;
            break;
    }

    return 0;
