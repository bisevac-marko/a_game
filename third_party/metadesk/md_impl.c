// LICENSE AT END OF FILE (MIT).

#define MD_FUNCTION_IMPL MD_FUNCTION
#define MD_PRIVATE_FUNCTION_IMPL MD_FUNCTION_IMPL
#define MD_UNTERMINATED_TOKEN_LEN_CAP 20

//~

// NOTE(allen): Review @rjf; Building in C++
// While very latest version of C++ have designated initializers
// I would like to be able to build on more simple versions, so I
// ditched the designated initializers in favor of the extra work
// of maintaining order based initializers.
static MD_Node _md_nil_node =
{
    &_md_nil_node,         // next
    &_md_nil_node,         // prev
    &_md_nil_node,         // parent
    &_md_nil_node,         // first_child
    &_md_nil_node,         // last_child
    &_md_nil_node,         // first_tag
    &_md_nil_node,         // last_tag
    MD_NodeKind_Nil,       // kind
    0,                     // flags
    MD_ZERO_STRUCT,        // string
    MD_ZERO_STRUCT,        // whole_string
    0xdeadffffffffffull,   // string_hash
    MD_ZERO_STRUCT,        // comment_before
    MD_ZERO_STRUCT,        // comment_after
    {(MD_u8*)"`NIL DD NODE`", 13}, // filename
    0,                     // file_contents
    0,                     // at
    &_md_nil_node,         // ref_target
};

//~ Memory Operations
MD_PRIVATE_FUNCTION_IMPL void
MD_MemoryZero(void *memory, MD_u64 size)
{
    memset(memory, 0, size);
}

MD_PRIVATE_FUNCTION_IMPL void
MD_MemoryCopy(void *dest, void *src, MD_u64 size)
{
    memcpy(dest, src, size);
}

MD_PRIVATE_FUNCTION_IMPL void *
MD_AllocZero(MD_u64 size)
{
#if !defined(MD_IMPL_Alloc)
# error Missing implementation detail MD_IMPL_Alloc
#else
    void *result = MD_IMPL_Alloc(size);
    MD_MemoryZero(result, size);
    return(result);
#endif
}

//~ Characters

MD_FUNCTION_IMPL MD_b32
MD_CharIsAlpha(MD_u8 c)
{
    return MD_CharIsAlphaUpper(c) || MD_CharIsAlphaLower(c);
}

MD_FUNCTION_IMPL MD_b32
MD_CharIsAlphaUpper(MD_u8 c)
{
    return c >= 'A' && c <= 'Z';
}

MD_FUNCTION_IMPL MD_b32
MD_CharIsAlphaLower(MD_u8 c)
{
    return c >= 'a' && c <= 'z';
}

MD_FUNCTION_IMPL MD_b32
MD_CharIsDigit(MD_u8 c)
{
    return (c >= '0' && c <= '9');
}

MD_FUNCTION_IMPL MD_b32
MD_CharIsSymbol(MD_u8 c)
{
    return (c == '~' || c == '!' || c == '@' || c == '#' || c == '$' ||
            c == '%' || c == '^' || c == '&' || c == '*' || c == '(' ||
            c == ')' || c == '-' || c == '=' || c == '+' || c == '[' ||
            c == ']' || c == '{' || c == '}' || c == ':' || c == ';' ||
            c == ',' || c == '<' || c == '.' || c == '>' || c == '/' ||
            c == '?' || c == '|' || c == '\\');
}

MD_FUNCTION_IMPL MD_b32
MD_CharIsReservedSymbol(MD_u8 c)
{
    return (c == '{' || c == '}' || c == '(' || c == ')' || c == '\\' ||
            c == '[' || c == ']' || c == '#' || c == ',' || c == ';'  ||
            c == ':' || c == '@');
}

MD_FUNCTION_IMPL MD_b32
MD_CharIsSpace(MD_u8 c)
{
    return c == ' ' || c == '\r' || c == '\t' || c == '\f' || c == '\v';
}

MD_FUNCTION_IMPL MD_u8
MD_CharToUpper(MD_u8 c)
{
    return (c >= 'a' && c <= 'z') ? ('A' + (c - 'a')) : c;
}

MD_FUNCTION_IMPL MD_u8
MD_CharToLower(MD_u8 c)
{
    return (c >= 'A' && c <= 'Z') ? ('a' + (c - 'A')) : c;
}

MD_FUNCTION_IMPL MD_u8
MD_CorrectSlash(MD_u8 c)
{
    return (c == '\\' ? '/' : c);
}

//~ Strings

MD_FUNCTION_IMPL MD_String8
MD_S8(MD_u8 *str, MD_u64 size)
{
    MD_String8 string;
    string.str = str;
    string.size = size;
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_S8Range(MD_u8 *str, MD_u8 *opl)
{
    MD_String8 string;
    string.str = str;
    string.size = (MD_u64)(opl - str);
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_StringSubstring(MD_String8 str, MD_u64 min, MD_u64 max)
{
    if(max > str.size)
    {
        max = str.size;
    }
    if(min > str.size)
    {
        min = str.size;
    }
    if(min > max)
    {
        MD_u64 swap = min;
        min = max;
        max = swap;
    }
    str.size = max - min;
    str.str += min;
    return str;
}

MD_FUNCTION_IMPL MD_String8
MD_StringSkip(MD_String8 str, MD_u64 min)
{
    return MD_StringSubstring(str, min, str.size);
}

MD_FUNCTION_IMPL MD_String8
MD_StringChop(MD_String8 str, MD_u64 nmax)
{
    return MD_StringSubstring(str, 0, str.size - nmax);
}

MD_FUNCTION_IMPL MD_String8
MD_StringPrefix(MD_String8 str, MD_u64 size)
{
    return MD_StringSubstring(str, 0, size);
}

MD_FUNCTION_IMPL MD_String8
MD_StringSuffix(MD_String8 str, MD_u64 size)
{
    return MD_StringSubstring(str, str.size - size, str.size);
}

MD_FUNCTION_IMPL MD_b32
MD_StringMatch(MD_String8 a, MD_String8 b, MD_MatchFlags flags)
{
    int result = 0;
    if(a.size == b.size || flags & MD_MatchFlag_RightSideSloppy)
    {
        result = 1;
        for(MD_u64 i = 0; i < a.size; i += 1)
        {
            MD_b32 match = (a.str[i] == b.str[i]);
            if(flags & MD_MatchFlag_CaseInsensitive)
            {
                match |= (MD_CharToLower(a.str[i]) == MD_CharToLower(b.str[i]));
            }
            if(flags & MD_MatchFlag_SlashInsensitive)
            {
                match |= (MD_CorrectSlash(a.str[i]) == MD_CorrectSlash(b.str[i]));
            }
            if(match == 0)
            {
                result = 0;
                break;
            }
        }
    }
    return result;
}

MD_FUNCTION_IMPL MD_u64
MD_FindSubstring(MD_String8 str, MD_String8 substring, MD_u64 start_pos, MD_MatchFlags flags)
{
    MD_b32 found = 0;
    MD_u64 found_idx = str.size;
    for(MD_u64 i = start_pos; i < str.size; i += 1)
    {
        if(i + substring.size <= str.size)
        {
            MD_String8 substr_from_str = MD_StringSubstring(str, i, i+substring.size);
            if(MD_StringMatch(substr_from_str, substring, flags))
            {
                found_idx = i;
                found = 1;
                if(!(flags & MD_MatchFlag_FindLast))
                {
                    break;
                }
            }
        }
    }
    return found_idx;
}

MD_FUNCTION_IMPL MD_String8
MD_ChopExtension(MD_String8 string)
{
    MD_u64 period_pos = MD_FindSubstring(string, MD_S8Lit("."), 0, MD_MatchFlag_FindLast);
    if(period_pos < string.size)
    {
        string.size = period_pos;
    }
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_SkipFolder(MD_String8 string)
{
    MD_u64 slash_pos = MD_FindSubstring(string, MD_S8Lit("/"), 0,
                                        MD_MatchFlag_SlashInsensitive|
                                        MD_MatchFlag_FindLast);
    if(slash_pos < string.size)
    {
        string.str += slash_pos+1;
        string.size -= slash_pos+1;
    }
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_ExtensionFromPath(MD_String8 string)
{
    MD_u64 period_pos = MD_FindSubstring(string, MD_S8Lit("."), 0, MD_MatchFlag_FindLast);
    if(period_pos < string.size)
    {
        string.str += period_pos+1;
        string.size -= period_pos+1;
    }
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_FolderFromPath(MD_String8 string)
{
    MD_u64 slash_pos = MD_FindSubstring(string, MD_S8Lit("/"), 0,
                                        MD_MatchFlag_SlashInsensitive|
                                        MD_MatchFlag_FindLast);
    if(slash_pos < string.size)
    {
        string.size = slash_pos;
    }
    return string;
}

MD_FUNCTION_IMPL MD_String8
MD_PushStringCopy(MD_String8 string)
{
    MD_String8 res;
    res.size = string.size;
    res.str = MD_PushArray(MD_u8, string.size + 1);
    MD_MemoryCopy(res.str, string.str, string.size);
    return(res);
}

MD_FUNCTION_IMPL MD_String8
MD_PushStringFV(char *fmt, va_list args)
{
    MD_String8 result = MD_ZERO_STRUCT;
    va_list args2;
    va_copy(args2, args);
    MD_u64 needed_bytes = vsnprintf(0, 0, fmt, args)+1;
    result.str = MD_PushArray(MD_u8, needed_bytes);
    result.size = needed_bytes-1;
    vsnprintf((char*)result.str, needed_bytes, fmt, args2);
    return result;
}

MD_FUNCTION_IMPL MD_String8
MD_PushStringF(char *fmt, ...)
{
    MD_String8 result = MD_ZERO_STRUCT;
    va_list args;
    va_start(args, fmt);
    result = MD_PushStringFV(fmt, args);
    va_end(args);
    return result;
}

MD_FUNCTION_IMPL void
MD_PushStringToList(MD_String8List *list, MD_String8 string)
{
    list->node_count += 1;
    list->total_size += string.size;
    
    MD_String8Node *node = MD_PushArray(MD_String8Node, 1);
    node->next = 0;
    node->string = string;
    if(list->last == 0)
    {
        list->first = list->last = node;
    }
    else
    {
        list->last->next = node;
        list->last = list->last->next;
    }
}

MD_FUNCTION_IMPL void
MD_PushStringListToList(MD_String8List *list, MD_String8List *to_push)
{
    if(to_push->first)
    {
        list->node_count += to_push->node_count;
        list->total_size += to_push->total_size;
        
        if(list->last == 0)
        {
            *list = *to_push;
        }
        else
        {
            list->last->next = to_push->first;
            list->last = to_push->last;
        }
    }
    MD_MemoryZero(to_push, sizeof(*to_push));
}

MD_FUNCTION_IMPL MD_String8List
MD_SplitString(MD_String8 string, int split_count, MD_String8 *splits)
{
    MD_String8List list = MD_ZERO_STRUCT;
    
    MD_u64 split_start = 0;
    for(MD_u64 i = 0; i < string.size; i += 1)
    {
        MD_b32 was_split = 0;
        for(int split_idx = 0; split_idx < split_count; split_idx += 1)
        {
            MD_b32 match = 0;
            if(i + splits[split_idx].size <= string.size)
            {
                match = 1;
                for(MD_u64 split_i = 0; split_i < splits[split_idx].size && i + split_i < string.size; split_i += 1)
                {
                    if(splits[split_idx].str[split_i] != string.str[i + split_i])
                    {
                        match = 0;
                        break;
                    }
                }
            }
            if(match)
            {
                MD_String8 split_string = MD_S8(string.str + split_start, i - split_start);
                MD_PushStringToList(&list, split_string);
                split_start = i + splits[split_idx].size;
                i += splits[split_idx].size - 1;
                was_split = 1;
                break;
            }
        }
        
        if(was_split == 0 && i == string.size - 1)
        {
            MD_String8 split_string = MD_S8(string.str + split_start, i+1 - split_start);
            MD_PushStringToList(&list, split_string);
            break;
        }
    }
    
    return list;
}

MD_FUNCTION_IMPL MD_String8
MD_JoinStringList(MD_String8List list, MD_String8 separator)
{
    if (list.node_count == 0)
    {
        return MD_S8Lit("");
    }
    MD_String8 string = MD_ZERO_STRUCT;
    string.size = list.total_size + (list.node_count - 1)*separator.size;
    string.str = MD_PushArray(MD_u8, string.size);
    MD_u64 write_pos = 0;
    for(MD_String8Node *node = list.first; node; node = node->next)
    {
        MD_MemoryCopy(string.str + write_pos, node->string.str, node->string.size);
        write_pos += node->string.size;
        if (node != list.last){
            MD_MemoryCopy(string.str + write_pos, separator.str, separator.size);
            write_pos += separator.size;
        }
    }
    return string;
}

MD_FUNCTION_IMPL MD_i64
MD_I64FromString(MD_String8 string, MD_u32 radix)
{
    MD_Assert(2 <= radix && radix <= 16);
    
    static MD_u8 char_to_value[] = {
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
        0x08,0x09,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    };
    
    // get the sign
    MD_i64 sign = +1;
    MD_u64 i =  0;
    if (string.size > 0){
        if (string.str[i] == '-'){
            i = 1;
            sign = -1;
        }
        if (string.str[i] == '+'){
            i = 1;
        }
    }
    
    // get the value
    MD_i64 value = 0;
    for (;i < string.size; i += 1){
        value *= radix;
        MD_u8 c = string.str[i];
        value += char_to_value[(c - 0x30)&0x1F];
    }
    value *= sign;
    
    return(value);
}

MD_FUNCTION_IMPL MD_f64
MD_F64FromString(MD_String8 string)
{
    char str[64];
    MD_u64 str_size = string.size;
    if (str_size > sizeof(str) - 1){
        str_size = sizeof(str) - 1;
    }
    MD_MemoryCopy(str, string.str, str_size);
    str[str_size] = 0;
    return(atof(str));
}

MD_FUNCTION_IMPL MD_u64
MD_HashString(MD_String8 string)
{
    MD_u64 result = 5381;
    for(MD_u64 i = 0; i < string.size; i += 1)
    {
        result = ((result << 5) + result) + string.str[i];
    }
    return result;
}

MD_FUNCTION_IMPL MD_u64
MD_CalculateCStringLength(char *cstr)
{
    MD_u64 i = 0;
    for(; cstr[i]; i += 1);
    return i;
}

MD_FUNCTION_IMPL MD_String8
MD_StyledStringFromString(MD_String8 string, MD_WordStyle word_style, MD_String8 separator)
{
    MD_String8 result = MD_ZERO_STRUCT;
    
    MD_String8List words = MD_ZERO_STRUCT;
    
    MD_b32 break_on_uppercase = 0;
    {
        break_on_uppercase = 1;
        for(MD_u64 i = 0; i < string.size; i += 1)
        {
            if(!MD_CharIsAlpha(string.str[i]) && !MD_CharIsDigit(string.str[i]))
            {
                break_on_uppercase = 0;
                break;
            }
        }
    }
    
    MD_b32 making_word = 0;
    MD_String8 word = MD_ZERO_STRUCT;
    
    for(MD_u64 i = 0; i < string.size;)
    {
        if(making_word)
        {
            if((break_on_uppercase && MD_CharIsAlphaUpper(string.str[i])) ||
               string.str[i] == '_' || MD_CharIsSpace(string.str[i]) ||
               i == string.size - 1)
            {
                if(i == string.size - 1)
                {
                    word.size += 1;
                }
                making_word = 0;
                MD_PushStringToList(&words, word);
            }
            else
            {
                word.size += 1;
                i += 1;
            }
        }
        else
        {
            if(MD_CharIsAlpha(string.str[i]))
            {
                making_word = 1;
                word.str = string.str + i;
                word.size = 1;
            }
            i += 1;
        }
    }
    
    result.size = words.total_size;
    if(words.node_count > 1)
    {
        result.size += separator.size*(words.node_count-1);
    }
    result.str = MD_PushArray(MD_u8, result.size);
    
    {
        MD_u64 write_pos = 0;
        for(MD_String8Node *node = words.first; node; node = node->next)
        {
            
            // NOTE(rjf): Write word string to result.
            {
                MD_MemoryCopy(result.str + write_pos, node->string.str, node->string.size);
                
                // NOTE(rjf): Transform string based on word style.
                switch(word_style)
                {
                    case MD_WordStyle_UpperCamelCase:
                    {
                        result.str[write_pos] = MD_CharToUpper(result.str[write_pos]);
                        for(MD_u64 i = write_pos+1; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = MD_CharToLower(result.str[i]);
                        }
                    }break;
                    
                    case MD_WordStyle_LowerCamelCase:
                    {
                        result.str[write_pos] = node == words.first ? MD_CharToLower(result.str[write_pos]) : MD_CharToUpper(result.str[write_pos]);
                        for(MD_u64 i = write_pos+1; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = MD_CharToLower(result.str[i]);
                        }
                    }break;
                    
                    case MD_WordStyle_UpperCase:
                    {
                        for(MD_u64 i = write_pos; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = MD_CharToUpper(result.str[i]);
                        }
                    }break;
                    
                    case MD_WordStyle_LowerCase:
                    {
                        for(MD_u64 i = write_pos; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = MD_CharToLower(result.str[i]);
                        }
                    }break;
                    
                    default: break;
                }
                
                write_pos += node->string.size;
            }
            
            if(node->next)
            {
                MD_MemoryCopy(result.str + write_pos, separator.str, separator.size);
                write_pos += separator.size;
            }
        }
    }
    
    return result;
}

//~ Enum/Flag Strings

MD_FUNCTION_IMPL MD_String8
MD_StringFromNodeKind(MD_NodeKind kind)
{
    // NOTE(rjf): Must be kept in sync with MD_NodeKind enum.
    static char *cstrs[MD_NodeKind_MAX] =
    {
        "Nil",
        "File",
        "List",
        "Reference",
        "Label",
        "Tag",
        "ErrorMarker",
    };
    return MD_S8CString(cstrs[kind]);
}

MD_FUNCTION_IMPL MD_String8List
MD_StringListFromNodeFlags(MD_NodeFlags flags)
{
    // NOTE(rjf): Must be kept in sync with MD_NodeFlags enum.
    static char *flag_cstrs[] =
    {
        "ParenLeft",
        "ParenRight",
        "BracketLeft",
        "BracketRight",
        "BraceLeft",
        "BraceRight",
        
        "BeforeSemicolon",
        "BeforeComma",
        
        "AfterSemicolon",
        "AfterComma",
        
        "Numeric",
        "Identifier",
        "StringLiteral",
        "CharLiteral",
    };
    
    MD_String8List list = MD_ZERO_STRUCT;
    MD_u64 bits = sizeof(flags) * 8;
    for(MD_u64 i = 0; i < bits && i < MD_ArrayCount(flag_cstrs); i += 1)
    {
        if(flags & (1ull << i))
        {
            MD_PushStringToList(&list, MD_S8CString(flag_cstrs[i]));
        }
    }
    return list;
}

//~ Unicode Conversions

MD_GLOBAL MD_u8 md_utf8_class[32] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
};

MD_FUNCTION_IMPL MD_UnicodeConsume
MD_CodepointFromUtf8(MD_u8 *str, MD_u64 max)
{
#define MD_bitmask1 0x01
#define MD_bitmask2 0x03
#define MD_bitmask3 0x07
#define MD_bitmask4 0x0F
#define MD_bitmask5 0x1F
#define MD_bitmask6 0x3F
#define MD_bitmask7 0x7F
#define MD_bitmask8 0xFF
#define MD_bitmask9  0x01FF
#define MD_bitmask10 0x03FF
    
    MD_UnicodeConsume result = {~((MD_u32)0), 1};
    MD_u8 byte = str[0];
    MD_u8 byte_class = md_utf8_class[byte >> 3];
    switch (byte_class)
    {
        case 1:
        {
            result.codepoint = byte;
        }break;
        
        case 2:
        {
            if (2 <= max)
            {
                MD_u8 cont_byte = str[1];
                if (md_utf8_class[cont_byte >> 3] == 0)
                {
                    result.codepoint = (byte & MD_bitmask5) << 6;
                    result.codepoint |=  (cont_byte & MD_bitmask6);
                    result.advance = 2;
                }
            }
        }break;
        
        case 3:
        {
            if (3 <= max)
            {
                MD_u8 cont_byte[2] = {str[1], str[2]};
                if (md_utf8_class[cont_byte[0] >> 3] == 0 &&
                    md_utf8_class[cont_byte[1] >> 3] == 0)
                {
                    result.codepoint = (byte & MD_bitmask4) << 12;
                    result.codepoint |= ((cont_byte[0] & MD_bitmask6) << 6);
                    result.codepoint |=  (cont_byte[1] & MD_bitmask6);
                    result.advance = 3;
                }
            }
        }break;
        
        case 4:
        {
            if (4 <= max)
            {
                MD_u8 cont_byte[3] = {str[1], str[2], str[3]};
                if (md_utf8_class[cont_byte[0] >> 3] == 0 &&
                    md_utf8_class[cont_byte[1] >> 3] == 0 &&
                    md_utf8_class[cont_byte[2] >> 3] == 0)
                {
                    result.codepoint = (byte & MD_bitmask3) << 18;
                    result.codepoint |= ((cont_byte[0] & MD_bitmask6) << 12);
                    result.codepoint |= ((cont_byte[1] & MD_bitmask6) <<  6);
                    result.codepoint |=  (cont_byte[2] & MD_bitmask6);
                    result.advance = 4;
                }
            }
        }break;
    }
    
    return(result);
}

MD_FUNCTION_IMPL MD_UnicodeConsume
MD_CodepointFromUtf16(MD_u16 *out, MD_u64 max)
{
    MD_UnicodeConsume result = {~((MD_u32)0), 1};
    result.codepoint = out[0];
    result.advance = 1;
    if (1 < max && 0xD800 <= out[0] && out[0] < 0xDC00 && 0xDC00 <= out[1] && out[1] < 0xE000)
    {
        result.codepoint = ((out[0] - 0xD800) << 10) | (out[1] - 0xDC00);
        result.advance = 2;
    }
    return(result);
}

MD_FUNCTION MD_u32
MD_Utf8FromCodepoint(MD_u8 *out, MD_u32 codepoint)
{
#define MD_bit8 0x80
    MD_u32 advance = 0;
    if (codepoint <= 0x7F)
    {
        out[0] = (MD_u8)codepoint;
        advance = 1;
    }
    else if (codepoint <= 0x7FF)
    {
        out[0] = (MD_bitmask2 << 6) | ((codepoint >> 6) & MD_bitmask5);
        out[1] = MD_bit8 | (codepoint & MD_bitmask6);
        advance = 2;
    }
    else if (codepoint <= 0xFFFF)
    {
        out[0] = (MD_bitmask3 << 5) | ((codepoint >> 12) & MD_bitmask4);
        out[1] = MD_bit8 | ((codepoint >> 6) & MD_bitmask6);
        out[2] = MD_bit8 | ( codepoint       & MD_bitmask6);
        advance = 3;
    }
    else if (codepoint <= 0x10FFFF)
    {
        out[0] = (MD_bitmask4 << 3) | ((codepoint >> 18) & MD_bitmask3);
        out[1] = MD_bit8 | ((codepoint >> 12) & MD_bitmask6);
        out[2] = MD_bit8 | ((codepoint >>  6) & MD_bitmask6);
        out[3] = MD_bit8 | ( codepoint        & MD_bitmask6);
        advance = 4;
    }
    else
    {
        out[0] = '?';
        advance = 1;
    }
    return(advance);
}

MD_FUNCTION MD_u32
MD_Utf16FromCodepoint(MD_u16 *out, MD_u32 codepoint)
{
    MD_u32 advance = 1;
    if (codepoint == ~((MD_u32)0))
    {
        out[0] = (MD_u16)'?';
    }
    else if (codepoint < 0x10000)
    {
        out[0] = (MD_u16)codepoint;
    }
    else
    {
        MD_u64 v = codepoint - 0x10000;
        out[0] = 0xD800 + (v >> 10);
        out[1] = 0xDC00 + (v & MD_bitmask10);
        advance = 2;
    }
    return(advance);
}

MD_FUNCTION MD_String8
MD_S8FromS16(MD_String16 in)
{
    MD_u64 cap = in.size*3;
    MD_u8 *str = MD_PushArray(MD_u8, cap + 1);
    MD_u16 *ptr = in.str;
    MD_u16 *opl = ptr + in.size;
    MD_u64 size = 0;
    MD_UnicodeConsume consume;
    for (;ptr < opl;)
    {
        consume = MD_CodepointFromUtf16(ptr, opl - ptr);
        ptr += consume.advance;
        size += MD_Utf8FromCodepoint(str + size, consume.codepoint);
    }
    str[size] = 0;
    return(MD_S8(str, size));
}

MD_FUNCTION MD_String16
MD_S16FromS8(MD_String8 in)
{
    MD_u64 cap = in.size*2;
    MD_u16 *str = MD_PushArray(MD_u16, (cap + 1));
    MD_u8 *ptr = in.str;
    MD_u8 *opl = ptr + in.size;
    MD_u64 size = 0;
    MD_UnicodeConsume consume;
    for (;ptr < opl;)
    {
        consume = MD_CodepointFromUtf8(ptr, opl - ptr);
        ptr += consume.advance;
        size += MD_Utf16FromCodepoint(str + size, consume.codepoint);
    }
    str[size] = 0;
    MD_String16 result = {str, size};
    return(result);
}

MD_FUNCTION MD_String8
MD_S8FromS32(MD_String32 in)
{
    MD_u64 cap = in.size*4;
    MD_u8 *str = MD_PushArray(MD_u8, cap + 1);
    MD_u32 *ptr = in.str;
    MD_u32 *opl = ptr + in.size;
    MD_u64 size = 0;
    MD_UnicodeConsume consume;
    for (;ptr < opl; ptr += 1)
    {
        size += MD_Utf8FromCodepoint(str + size, *ptr);
    }
    str[size] = 0;
    return(MD_S8(str, size));
}

MD_FUNCTION MD_String32
MD_S32FromS8(MD_String8 in)
{
    MD_u64 cap = in.size;
    MD_u32 *str = MD_PushArray(MD_u32, (cap + 1));
    MD_u8 *ptr = in.str;
    MD_u8 *opl = ptr + in.size;
    MD_u64 size = 0;
    MD_UnicodeConsume consume;
    for (;ptr < opl;)
    {
        consume = MD_CodepointFromUtf8(ptr, opl - ptr);
        ptr += consume.advance;
        str[size] = consume.codepoint;
        size += 1;
    }
    str[size] = 0;
    MD_String32 result = {str, size};
    return(result);
}

//~ Map Table Data Structure

MD_PRIVATE_FUNCTION_IMPL void
_MD_Map_Initialize(MD_Map *map)
{
    if(map->table_size == 0)
    {
        map->table_size = 4096;
        map->table = MD_PushArray(MD_MapSlot *, map->table_size);
    }
}

// NOTE(mal): Generic 64-bit hash function (https://nullprogram.com/blog/2018/07/31/)
//            Reversible, so no collisions. Assumes all bits of the pointer matter.
MD_FUNCTION_IMPL MD_u64 
MD_HashPointer(void *p)
{
    MD_u64 h = (MD_u64)p;
    h = (h ^ (h >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    h = (h ^ (h >> 27)) * UINT64_C(0x94d049bb133111eb);
    h = h ^ (h >> 31);
    return h;
}

//- String-To-Pointer Table

MD_FUNCTION_IMPL MD_MapSlot *
MD_StringMap_Lookup(MD_Map *map, MD_String8 string)
{
    _MD_Map_Initialize(map);
    MD_MapSlot *slot = 0;
    MD_u64 hash = MD_HashString(string);
    MD_u64 index = hash % map->table_size;
    for(MD_MapSlot *candidate = map->table[index]; candidate; candidate = candidate->next)
    {
        if(candidate->hash == hash && MD_StringMatch(*((MD_String8 *)candidate->key), string, 0))
        {
            slot = candidate;
            break;
        }
    }
    return slot;
}

MD_FUNCTION_IMPL MD_b32
MD_StringMap_Insert(MD_Map *map, MD_MapCollisionRule collision_rule, MD_String8 string, void *value)
{
    _MD_Map_Initialize(map);
    
    MD_MapSlot *slot = 0;
    MD_u64 hash = MD_HashString(string);
    MD_u64 index = hash % map->table_size;
    
    for(MD_MapSlot *candidate = map->table[index]; candidate; candidate = candidate->next)
    {
        if(candidate->hash == hash && MD_StringMatch(*((MD_String8 *)candidate->key), string, 0))
        {
            slot = candidate;
            break;
        }
    }
    
    if(slot == 0 || (slot != 0 && collision_rule == MD_MapCollisionRule_Chain))
    {
        slot = MD_PushArray(MD_MapSlot, 1);
        if(slot)
        {
            slot->next = 0;
            if(map->table[index])
            {
                for(MD_MapSlot *old_slot = map->table[index]; old_slot; old_slot = old_slot->next)
                {
                    if(old_slot->next == 0)
                    {
                        old_slot->next = slot;
                        break;
                    }
                }
            }
            else
            {
                map->table[index] = slot;
            }
        }
    }
    
    if(slot)
    {
        slot->value = value;
        slot->hash = hash;
        MD_String8 *string_copy =  MD_PushArray(MD_String8, 1);
        *string_copy = MD_PushStringCopy(string);
        slot->key = string_copy;
    }
    
    return !!slot;
}

MD_FUNCTION_IMPL MD_MapSlot *
MD_StringMap_Next(MD_MapSlot *slot, MD_String8 key)
{
    MD_MapSlot *next = 0;
    if(slot)
    {
        for(MD_MapSlot *candidate = slot->next; candidate; candidate = candidate->next)
        {
            if(MD_StringMatch(*(MD_String8 *)candidate->key, key, 0))
            {
                next = candidate;
                break;
            }
        }
    }
    return next;
}

//- Pointer-To-Pointer Table

MD_FUNCTION_IMPL MD_MapSlot *
MD_PtrMap_Lookup(MD_Map *map, void *key)
{
    _MD_Map_Initialize(map);
    
    MD_MapSlot *slot = 0;
    MD_u64 hash = MD_HashPointer(key);
    MD_u64 index = hash % map->table_size;
    for(MD_MapSlot *candidate = map->table[index]; candidate; candidate = candidate->next)
    {
        if(candidate->hash == hash)
        {
            slot = candidate;
            break;
        }
    }
    
    return slot;
}

MD_FUNCTION_IMPL MD_b32
MD_PtrMap_Insert(MD_Map *map, MD_MapCollisionRule collision_rule, void *key, void *value)
{
    _MD_Map_Initialize(map);
    
    MD_MapSlot *slot = 0;
    MD_u64 hash = MD_HashPointer(key);
    MD_u64 index = hash % map->table_size;
    
    for(MD_MapSlot *candidate = map->table[index]; candidate; candidate = candidate->next)
    {
        if(candidate->hash == hash)
        {
            slot = candidate;
            break;
        }
    }
    
    if(slot == 0 || (slot != 0 && collision_rule == MD_MapCollisionRule_Chain))
    {
        slot = MD_PushArray(MD_MapSlot, 1);
        if(slot)
        {
            slot->next = 0;
            if(map->table[index])
            {
                for(MD_MapSlot *old_slot = map->table[index]; old_slot; old_slot = old_slot->next)
                {
                    if(old_slot->next == 0)
                    {
                        old_slot->next = slot;
                        break;
                    }
                }
            }
            else
            {
                map->table[index] = slot;
            }
        }
    }
    
    if(slot)
    {
        slot->value = value;
        slot->hash = hash;
    }
    
    return !!slot;
}

//~ Parsing

MD_PRIVATE_FUNCTION_IMPL void
_MD_Error(MD_ParseCtx *ctx, MD_Node *node, MD_MessageKind kind, char *fmt, ...)
{
    // NOTE(mal): Sort errors. Traverse the whole list assuming it will be short.
    //            The alternative is to drop a prev pointer into MD_Error and search backwards
    MD_Error *prev_error = 0;
    for(MD_Error *e = ctx->first_error; e; e = e->next)
    {
        if(e->node->at < node->at)
        {
            prev_error = e;
        }
        else
        {
            break;
        }
    }
    
    // NOTE(mal): Ignore errors after first catastrophic error
    if(ctx->error_level < MD_MessageKind_CatastrophicError || !prev_error || prev_error->next)
    {
        MD_Error *error = MD_PushArray(MD_Error, 1);
        error->node = node;
        error->kind = kind;
        va_list args;
        va_start(args, fmt);
        error->string = MD_PushStringFV(fmt, args);
        va_end(args);
        
        if(prev_error)
        {
            error->next = prev_error->next;
            prev_error->next = error;
        }
        else
        {
            error->next = ctx->first_error;
            ctx->first_error = error;
        }
        
        if(!ctx->last_error || ctx->last_error == prev_error)
        {
            ctx->last_error = error;
        }
        
        if(kind > ctx->error_level)
        {
            ctx->error_level = kind;
        }
    }
}
#define _MD_TokenError(ctx, token, kind, fmt, ...) \
_MD_Error(ctx, _MD_MakeNodeFromToken_Ctx(ctx, MD_NodeKind_ErrorMarker, token), kind, fmt, __VA_ARGS__)

MD_PRIVATE_FUNCTION_IMPL MD_Node *
_MD_MakeNodeFromToken_Ctx(MD_ParseCtx *ctx, MD_NodeKind kind, MD_Token token)
{
    return MD_MakeNode(kind, token.string, token.outer_string, ctx->filename,
                       ctx->file_contents.str,
                       token.outer_string.str);
}

MD_PRIVATE_FUNCTION_IMPL MD_Node *
_MD_MakeNodeFromString_Ctx(MD_ParseCtx *ctx, MD_NodeKind kind, MD_String8 string, MD_u8 *at)
{
    return MD_MakeNode(kind, string, string, ctx->filename, ctx->file_contents.str, at);
}

MD_PRIVATE_FUNCTION_IMPL void _MD_ParseTagList(MD_ParseCtx *ctx, MD_Node **first_out, MD_Node **last_out);

MD_PRIVATE_FUNCTION_IMPL MD_NodeFlags
_MD_NodeFlagsFromTokenKind(MD_TokenKind kind)
{
    MD_NodeFlags result = 0;
    switch (kind){
        case MD_TokenKind_Identifier:     result = MD_NodeFlag_Identifier;    break;
        case MD_TokenKind_NumericLiteral: result = MD_NodeFlag_Numeric;       break;
        case MD_TokenKind_StringLiteral:  result = MD_NodeFlag_StringLiteral; break;
        case MD_TokenKind_CharLiteral:    result = MD_NodeFlag_CharLiteral;   break;
    }
    return(result);
}

MD_PRIVATE_FUNCTION_IMPL MD_b32
_MD_StringLiteralIsBalanced(MD_Token token)
{
    MD_u64 front_len = token.string.str - token.outer_string.str;
    MD_u64 back_len  = (token.outer_string.str + token.outer_string.size) - (token.string.str + token.string.size);
    MD_b32 result = (front_len == back_len);
    return result;
}

MD_PRIVATE_FUNCTION_IMPL MD_b32
_MD_CommentIsSyntacticallyCorrect(MD_Token comment_token)
{
    MD_String8 inner = comment_token.string;
    MD_String8 outer = comment_token.outer_string;
    MD_b32 incorrect = (MD_StringMatch(MD_StringPrefix(outer, 2), MD_S8Lit("/*"), 0) &&   // C-style comment
                        (inner.str != outer.str + 2 || inner.str + inner.size != outer.str + outer.size - 2)); // Internally unbalanced
    MD_b32 result = !incorrect;
    return result;
}
MD_PRIVATE_FUNCTION_IMPL void
_MD_ParseTagList(MD_ParseCtx *ctx, MD_Node **first_out, MD_Node **last_out)
{
    MD_Node *first = MD_NilNode();
    MD_Node *last = MD_NilNode();
    
    for(;;)
    {
        MD_Token next_token = MD_Parse_PeekSkipSome(ctx, MD_TokenGroup_Comment | MD_TokenGroup_Whitespace);
        if(MD_StringMatch(next_token.string, MD_S8Lit("@"), 0) &&
           next_token.kind == MD_TokenKind_Symbol)
        {
            MD_Parse_Bump(ctx, next_token);
            
            MD_Token name = MD_ZERO_STRUCT;
            if(MD_Parse_RequireKind(ctx, MD_TokenKind_Identifier, &name))
            {
                MD_Node *tag = _MD_MakeNodeFromToken_Ctx(ctx, MD_NodeKind_Tag, name);
                MD_Token token = MD_Parse_PeekSkipSome(ctx, 0);
                if(MD_StringMatch(token.string, MD_S8Lit("("), 0))
                {
                    MD_Parse_Set(ctx, tag, MD_ParseSetFlag_Paren);
                }
                MD_PushSibling(&first, &last, tag);
            }
            else
            {
                MD_Token token = MD_Parse_PeekSkipSome(ctx, 0);
                _MD_TokenError(ctx, token, MD_MessageKind_Error, "\"%.*s\" is not a proper tag identifier",
                               MD_StringExpand(token.outer_string));
                // NOTE(mal): There are reasons to consume the non-tag token, but also to leave it.
                break;
            }
        }
        else
        {
            break;
        }
    }
    
    *first_out = first;
    *last_out = last;
}

MD_FUNCTION_IMPL MD_b32
MD_TokenKindIsWhitespace(MD_TokenKind kind)
{
    return kind > MD_TokenKind_WhitespaceMin && kind < MD_TokenKind_WhitespaceMax;
}

MD_FUNCTION_IMPL MD_b32
MD_TokenKindIsComment(MD_TokenKind kind)
{
    return(kind == MD_TokenKind_Comment);
}

MD_FUNCTION_IMPL MD_b32
MD_TokenKindIsRegular(MD_TokenKind kind)
{
    return(kind > MD_TokenKind_RegularMin && kind < MD_TokenKind_RegularMax);
}

MD_FUNCTION_IMPL MD_ParseCtx
MD_Parse_InitializeCtx(MD_String8 filename, MD_String8 contents)
{
    MD_ParseCtx ctx = MD_ZERO_STRUCT;
    ctx.at = contents.str;
    ctx.file_contents = contents;
    ctx.filename = filename;
    return ctx;
}

MD_FUNCTION_IMPL void
MD_Parse_Bump(MD_ParseCtx *ctx, MD_Token token)
{
    ctx->at = token.outer_string.str + token.outer_string.size;
}

MD_FUNCTION_IMPL void
MD_Parse_BumpNext(MD_ParseCtx *ctx)
{
    MD_Parse_Bump(ctx, MD_Parse_LexNext(ctx));
}

MD_PRIVATE_FUNCTION_IMPL MD_u8 *
_MD_TokenizerScanEscaped(MD_u8 *at, MD_u8 *one_past_last, MD_u8 c)
{
    while(at < one_past_last)
    {
        if(*at == c || *at == '\n') break;
        else if(at[0] == '\\' && at + 1 < one_past_last && (at[1] == c || at[1] == '\\')) at += 2;
        else at += 1;
    }
    
    if (at < one_past_last && *at == c) at += 1;
    
    return at;
}

MD_PRIVATE_FUNCTION_IMPL MD_u8 *
_MD_TokenizerScanTripletEscaped(MD_u8 *at, MD_u8 *one_past_last, MD_u8 c)
{
    MD_u32 consecutive_c = 0;
    while(at < one_past_last && consecutive_c < 3)
    {
        if(at[0] == c)
        {
            consecutive_c += 1;
        }
        else
        {
            consecutive_c = 0;
            if(at[0] == '\\' && at + 1 < one_past_last)
            {
                if(at[1] == c || at[1] == '\\')
                {
                    at += 1;
                }
            }
        }
        at += 1;
    }
    return at;
}

MD_PRIVATE_FUNCTION_IMPL MD_u32
_MD_ComputeTextLiteralChop(MD_u32 bytes_to_skip, MD_u8 *beg, MD_u8 *end)
{
    MD_u32 result = 0;
    MD_u8 *skip_end = beg + bytes_to_skip;
    MD_u8 *chop_beg = end - bytes_to_skip;
    if(skip_end <= chop_beg && MD_StringMatch(MD_S8(beg, bytes_to_skip), MD_S8(chop_beg, bytes_to_skip), 0))
    {
        result = bytes_to_skip;
    }
    return result;
}

MD_FUNCTION_IMPL MD_Token
MD_Parse_LexNext(MD_ParseCtx *ctx)
{
    MD_Token token;
    MD_MemoryZero(&token, sizeof(token));
    
    MD_u8 *one_past_last = ctx->file_contents.str + ctx->file_contents.size;
    MD_u8 *first = ctx->at;
    if (first < one_past_last)
    {
        MD_u8 *at = first;
        MD_u32 skip_n = 0;
        MD_u32 chop_n = 0;
        
#define MD_TokenizerScan(cond) for (; at < one_past_last && (cond); at += 1)
        
        switch (*at)
        {
            // NOTE(allen): Whitespace parsing
            case '\n':
            {
                token.kind = MD_TokenKind_Newline;
                at += 1;
            }break;
            
            case ' ': case '\r': case '\t': case '\f': case '\v':
            {
                token.kind = MD_TokenKind_Whitespace;
                at += 1;
                MD_TokenizerScan(*at == ' ' || *at == '\r' || *at == '\t' || *at == '\f' || *at == '\v');
            }break;
            
            // NOTE(allen): Comment parsing
            case '/':
            {
                if (at + 1 < one_past_last)
                {
                    if (at[1] == '/')
                    {
                        
                        // NOTE(rjf): Trim off the first //, and a space after it if one is there.
                        {
                            if(at+2 < one_past_last &&
                               at[2] == ' ')
                            {
                                skip_n = 3;
                            }
                            else
                            {
                                skip_n = 2;
                            }
                        }
                        
                        at += 2;
                        token.kind = MD_TokenKind_Comment;
                        MD_TokenizerScan(*at != '\n' && *at != '\r');
                    }
                    else if (at[1] == '*')
                    {
                        at += 2;
                        token.kind = MD_TokenKind_Comment;
                        skip_n = 2;
                        int counter = 1;
                        for (;at < one_past_last && counter > 0; at += 1)
                        {
                            if (at + 1 < one_past_last)
                            {
                                if (at[0] == '*' && at[1] == '/')
                                {
                                    at += 1;
                                    counter -= 1;
                                }
                                else if (at[0] == '/' && at[1] == '*')
                                {
                                    at += 1;
                                    counter += 1;
                                }
                            }
                        }
                        if(counter == 0)
                        {
                            chop_n = 2;
                        }
                    }
                }
                if (token.kind == MD_TokenKind_Nil) goto symbol_lex;
            }break;
            
            // NOTE(rjf): "Bundle-of-tokens" strings (`stuff` or ```stuff```)
            // In practice no different than a regular string, but provides an
            // alternate syntax which will allow tools like 4coder to treat the
            // contents as regular tokens.
            case '`':
            {
                token.kind = MD_TokenKind_StringLiteral;
                if (at + 2 < one_past_last && at[1] == '`' && at[2] == '`')
                {
                    skip_n = 3;
                    at = _MD_TokenizerScanTripletEscaped(at+3, one_past_last, '`');
                }
                else
                {
                    skip_n = 1;
                    at = _MD_TokenizerScanEscaped(at+1, one_past_last, '`');
                }
                chop_n = _MD_ComputeTextLiteralChop(skip_n, first, at);
            }break;
            
            // NOTE(allen): Strings
            case '"':
            {
                token.kind = MD_TokenKind_StringLiteral;
                if (at + 2 < one_past_last && at[1] == '"' && at[2] == '"')
                {
                    skip_n = 3;
                    at = _MD_TokenizerScanTripletEscaped(at+3, one_past_last, '"');
                }
                else
                {
                    skip_n = 1;
                    at += 1;
                    at = _MD_TokenizerScanEscaped(at, one_past_last, '"');
                }
                chop_n = _MD_ComputeTextLiteralChop(skip_n, first, at);
            }break;
            
            case '\'':
            {
                if (at + 2 < one_past_last && at[1] == '\'' && at[2] == '\'')
                {
                    token.kind = MD_TokenKind_StringLiteral;
                    skip_n = 3;
                    at = _MD_TokenizerScanTripletEscaped(at+3, one_past_last, '\'');
                }
                else
                {
                    token.kind = MD_TokenKind_CharLiteral;
                    skip_n = 1;
                    at += 1;
                    at = _MD_TokenizerScanEscaped(at, one_past_last, '\'');
                }
                chop_n = _MD_ComputeTextLiteralChop(skip_n, first, at);
            }break;
            
            // NOTE(allen): Identifiers, Numbers, Operators
            default:
            {
                if (MD_CharIsAlpha(*at) || *at == '_')
                {
                    token.kind = MD_TokenKind_Identifier;
                    at += 1;
                    MD_TokenizerScan(MD_CharIsAlpha(*at) || MD_CharIsDigit(*at) || *at == '_');
                }
                
                else if (MD_CharIsDigit(*at) ||
                         (at + 1 < one_past_last && at[0] == '-' && MD_CharIsDigit(at[1])))
                {
                    token.kind = MD_TokenKind_NumericLiteral;
                    at += 1;
                    MD_TokenizerScan(MD_CharIsAlpha(*at) || MD_CharIsDigit(*at) || *at == '.');
                }
                
                else if (MD_CharIsSymbol(*at))
                {
                    symbol_lex:
                    token.kind = MD_TokenKind_Symbol;
                    at += 1;
                }
                
                else
                {
                    token.kind = MD_TokenKind_BadCharacter;
                    at += 1;
                }
            }break;
        }
        
        token.outer_string = MD_S8Range(first, at);
        token.string = MD_StringSubstring(token.outer_string, skip_n, token.outer_string.size - chop_n);
        
        ctx->at = at;
    }
    
    return token;
}

MD_FUNCTION_IMPL MD_Token
MD_Parse_PeekSkipSome(MD_ParseCtx *ctx, MD_TokenGroups skip_groups)
{
    MD_ParseCtx ctx_restore = *ctx;
    
    MD_b32 skip_comment    = (skip_groups & MD_TokenGroup_Comment);
    MD_b32 skip_whitespace = (skip_groups & MD_TokenGroup_Whitespace);
    MD_b32 skip_regular    = (skip_groups & MD_TokenGroup_Regular);
    
    MD_Token result;
    MD_MemoryZero(&result, sizeof(result));
    
    loop:
    {
        result = MD_Parse_LexNext(ctx);
        if ((skip_comment    && MD_TokenKindIsComment(result.kind)) ||
            (skip_whitespace && MD_TokenKindIsWhitespace(result.kind)) ||
            (skip_regular    && MD_TokenKindIsRegular(result.kind))){
            MD_Parse_Bump(ctx, result);
            goto loop;
        }
    }
    
    {
        // TODO(allen): I'm not a fan of what this implies.
        *ctx = ctx_restore;
    }
    
    return result;
}

MD_FUNCTION_IMPL MD_b32
MD_Parse_TokenMatch(MD_Token token, MD_String8 string, MD_MatchFlags flags)
{
    return MD_StringMatch(token.string, string, flags);
}

MD_FUNCTION_IMPL MD_b32
MD_Parse_Require(MD_ParseCtx *ctx, MD_String8 string, MD_TokenKind kind)
{
    int result = 0;
    
    MD_Token token_any = MD_Parse_PeekSkipSome(ctx, 0);
    MD_Token token_regular;
    if(MD_StringMatch(token_any.string, string, 0) && token_any.kind == kind)
    {
        result = 1;
        MD_Parse_Bump(ctx, token_any);
        goto end;
    }
    
    token_regular = MD_Parse_PeekSkipSome(ctx, MD_TokenGroup_Comment|MD_TokenGroup_Whitespace);
    if(MD_StringMatch(token_regular.string, string, 0) && token_regular.kind == kind)
    {
        result = 1;
        MD_Parse_Bump(ctx, token_regular);
        goto end;
    }
    
    end:;
    return result;
}

MD_FUNCTION_IMPL MD_b32
MD_Parse_RequireKind(MD_ParseCtx *ctx, MD_TokenKind kind, MD_Token *out_token)
{
    int result = 0;
    
    MD_TokenGroups skip_groups = MD_TokenGroup_Comment|MD_TokenGroup_Whitespace;
    if (MD_TokenKindIsWhitespace(kind))
    {
        skip_groups &= ~MD_TokenGroup_Whitespace;
    }
    if (MD_TokenKindIsComment(kind))
    {
        skip_groups &= ~MD_TokenGroup_Comment;
    }
    
    MD_Token token = MD_Parse_PeekSkipSome(ctx, skip_groups);
    if(token.kind == kind)
    {
        result = 1;
        MD_Parse_Bump(ctx, token);
        if(out_token)
        {
            *out_token = token;
        }
    }
    return result;
}

MD_FUNCTION_IMPL void
MD_Parse_Set(MD_ParseCtx *ctx, MD_Node *parent, MD_ParseSetFlags flags)
{
    MD_b32 brace = 0;
    MD_b32 paren = 0;
    MD_b32 bracket = 0;
    MD_b32 terminate_with_separator = (!!(flags & MD_ParseSetFlag_Implicit));
    
    MD_Token initial_token = MD_Parse_PeekSkipSome(ctx, MD_TokenGroup_Comment|MD_TokenGroup_Whitespace);
    
    if((flags & MD_ParseSetFlag_Brace) &&
       MD_Parse_Require(ctx, MD_S8Lit("{"), MD_TokenKind_Symbol))
    {
        parent->flags |= MD_NodeFlag_BraceLeft;
        brace = 1;
        terminate_with_separator = 0;
    }
    else if((flags & MD_ParseSetFlag_Paren) &&
            MD_Parse_Require(ctx, MD_S8Lit("("), MD_TokenKind_Symbol))
    {
        parent->flags |= MD_NodeFlag_ParenLeft;
        paren = 1;
        terminate_with_separator = 0;
    }
    else if((flags & MD_ParseSetFlag_Bracket) &&
            MD_Parse_Require(ctx, MD_S8Lit("["), MD_TokenKind_Symbol))
    {
        parent->flags |= MD_NodeFlag_BracketLeft;
        bracket = 1;
        terminate_with_separator = 0;
    }
    
    // NOTE(rjf): Parse children.
    if(brace || paren || bracket || terminate_with_separator)
    {
        MD_u8 *at_before_children = ctx->at;
        MD_NodeFlags next_child_flags = 0;
        for(MD_u64 child_idx = 0;; child_idx += 1)
        {
            if(brace)
            {
                if(MD_Parse_Require(ctx, MD_S8Lit("}"), MD_TokenKind_Symbol))
                {
                    parent->flags |= MD_NodeFlag_BraceRight;
                    goto end_parse;
                }
            }
            else if(paren || bracket)
            {
                if((flags & MD_ParseSetFlag_Paren) &&
                   MD_Parse_Require(ctx, MD_S8Lit(")"), MD_TokenKind_Symbol))
                {
                    parent->flags |= MD_NodeFlag_ParenRight;
                    goto end_parse;
                }
                else if((flags & MD_ParseSetFlag_Bracket) &&
                        MD_Parse_Require(ctx, MD_S8Lit("]"), MD_TokenKind_Symbol))
                {
                    parent->flags |= MD_NodeFlag_BracketRight;
                    goto end_parse;
                }
            }
            else
            {
                MD_Token peek = MD_Parse_PeekSkipSome(ctx, MD_TokenGroup_Whitespace | MD_TokenGroup_Comment);
                if(peek.kind == MD_TokenKind_Symbol &&
                   (MD_Parse_TokenMatch(peek, MD_S8Lit("}"), 0) ||
                    MD_Parse_TokenMatch(peek, MD_S8Lit(")"), 0) ||
                    MD_Parse_TokenMatch(peek, MD_S8Lit("]"), 0)))
                {
                    goto end_parse;
                }
            }
            
            MD_ParseResult parse = MD_ParseOneNodeFromCtx(ctx);
            MD_Node *child = parse.node;
            child->flags |= next_child_flags;
            next_child_flags = 0;
            if(MD_NodeIsNil(child))
            {
                if(brace || paren || bracket)
                {
                    char delimiter_char = 0;
                    if(brace) delimiter_char = '{';
                    else if(paren) delimiter_char = '(';
                    else if(bracket) delimiter_char = '[';
                    _MD_TokenError(ctx, initial_token, MD_MessageKind_CatastrophicError, "Unbalanced \"%c\"", delimiter_char);
                }
                goto end_parse;
            }
            else
            {
                MD_PushSibling(&parent->first_child, &parent->last_child, child);
                child->parent = parent;
            }
            
            // NOTE(rjf): Separators.
            {
                MD_b32 result = 0;
                if(terminate_with_separator)
                {
                    MD_Token next_token = MD_Parse_PeekSkipSome(ctx, 0);
                    if(next_token.kind == MD_TokenKind_Newline ||
                       (next_token.kind == MD_TokenKind_Symbol &&
                        (MD_StringMatch(next_token.string, MD_S8Lit(","), 0) ||
                         MD_StringMatch(next_token.string, MD_S8Lit(";"), 0))))
                    {
                        result = 1;
                    }
                }
                else if(MD_Parse_Require(ctx, MD_S8Lit(","), MD_TokenKind_Symbol))
                {
                    child->flags |= MD_NodeFlag_BeforeComma;
                    next_child_flags |= MD_NodeFlag_AfterComma;
                }
                else if(MD_Parse_Require(ctx, MD_S8Lit(";"), MD_TokenKind_Symbol))
                {
                    child->flags |= MD_NodeFlag_BeforeSemicolon;
                    next_child_flags |= MD_NodeFlag_AfterSemicolon;
                }
                
                if(result)
                {
                    goto end_parse;
                }
            }
        }
    }
    
    end_parse:;
}

MD_FUNCTION_IMPL MD_ParseResult
MD_ParseOneNodeFromCtx(MD_ParseCtx *ctx)
{
    MD_u8 *at_first = ctx->at;
    
    MD_ParseResult result = MD_ZERO_STRUCT;
    result.node = MD_NilNode();
    
    MD_Error *ctx_last_error = ctx->last_error;
    
    MD_Token token;
    MD_MemoryZero(&token, sizeof(token));
    
    MD_Node *first_tag = 0;
    MD_Node *last_tag = 0;
    _MD_ParseTagList(ctx, &first_tag, &last_tag);
    
    // NOTE(rjf): Parse the comment preceding this node.
    MD_String8 comment_before = MD_ZERO_STRUCT;
    {
        MD_Token comment_token = MD_ZERO_STRUCT;
        for(;;)
        {
            MD_Token token = MD_Parse_PeekSkipSome(ctx, 0);
            if(token.kind == MD_TokenKind_Comment)
            {
                comment_token = token;
                MD_Parse_Bump(ctx, token);
            }
            else if(token.kind == MD_TokenKind_Newline)
            {
                MD_Parse_Bump(ctx, token);
                if(MD_Parse_RequireKind(ctx, MD_TokenKind_Comment, &comment_token))
                {
                    // NOTE(mal): If more than one comment, use the last comment
                }
                else if(MD_Parse_RequireKind(ctx, MD_TokenKind_Newline, 0))
                {
                    MD_MemoryZero(&comment_token, sizeof(comment_token));
                }
            }
            else if(MD_TokenKindIsWhitespace(token.kind))
            {
                MD_Parse_Bump(ctx, token);
            }
            else
            {
                break;
            }
        }
        comment_before = comment_token.string;
        if(!_MD_CommentIsSyntacticallyCorrect(comment_token))
        {
            MD_String8 capped = MD_StringPrefix(comment_token.outer_string, MD_UNTERMINATED_TOKEN_LEN_CAP);
            _MD_TokenError(ctx, comment_token, MD_MessageKind_CatastrophicError,
                           "Unterminated comment \"%.*s\"", MD_StringExpand(capped));
        }
    }
    
    MD_TokenGroups skip_groups = MD_TokenGroup_Whitespace|MD_TokenGroup_Comment;
    MD_Token next_token = MD_Parse_PeekSkipSome(ctx, skip_groups);
    
    retry:
    
    // NOTE(rjf): Unnamed Sets
    if((MD_Parse_TokenMatch(next_token, MD_S8Lit("("), 0) ||
        MD_Parse_TokenMatch(next_token, MD_S8Lit("{"), 0) ||
        MD_Parse_TokenMatch(next_token, MD_S8Lit("["), 0)) &&
       next_token.kind == MD_TokenKind_Symbol )
    {
        result.node = _MD_MakeNodeFromString_Ctx(ctx, MD_NodeKind_Label, MD_S8Lit(""), next_token.outer_string.str);
        
        MD_Parse_Set(ctx, result.node,
                     MD_ParseSetFlag_Paren   |
                     MD_ParseSetFlag_Brace   |
                     MD_ParseSetFlag_Bracket);
        goto end_parse;
    }
    
    // NOTE(rjf): Labels
    else if(MD_Parse_RequireKind(ctx, MD_TokenKind_Identifier,     &token) ||
            MD_Parse_RequireKind(ctx, MD_TokenKind_NumericLiteral, &token) ||
            MD_Parse_RequireKind(ctx, MD_TokenKind_StringLiteral,  &token) ||
            MD_Parse_RequireKind(ctx, MD_TokenKind_CharLiteral,    &token) ||
            MD_Parse_RequireKind(ctx, MD_TokenKind_Symbol,         &token))
    {
        result.node = MD_MakeNodeFromToken(MD_NodeKind_Label, ctx->filename, ctx->file_contents.str,
                                           token.outer_string.str, token);
        result.node->flags |= _MD_NodeFlagsFromTokenKind(token.kind);
        
        if(token.kind == MD_TokenKind_CharLiteral || token.kind == MD_TokenKind_StringLiteral)
        {
            if(!_MD_StringLiteralIsBalanced(token))
            {
                MD_String8 capped = MD_StringPrefix(token.outer_string, MD_UNTERMINATED_TOKEN_LEN_CAP);
                _MD_Error(ctx, result.node, MD_MessageKind_CatastrophicError,
                          "Unterminated text literal \"%.*s\"", MD_StringExpand(capped));
            }
        }
        else if(token.kind == MD_TokenKind_Symbol && token.string.size == 1 && MD_CharIsReservedSymbol(token.string.str[0]))
        {
            MD_u8 c = token.string.str[0];
            if(c == '}' || c == ']' || c == ')')
            {
                _MD_TokenError(ctx, token, MD_MessageKind_CatastrophicError, "Unbalanced \"%c\"", c);
            }
            else
            {
                _MD_TokenError(ctx, token, MD_MessageKind_Error, "Unexpected reserved symbol \"%c\"", c);
            }
        }
        
        // NOTE(rjf): Children
        if(MD_Parse_Require(ctx, MD_S8Lit(":"), MD_TokenKind_Symbol))
        {
            MD_Parse_Set(ctx, result.node,
                         MD_ParseSetFlag_Paren   |
                         MD_ParseSetFlag_Brace   |
                         MD_ParseSetFlag_Bracket |
                         MD_ParseSetFlag_Implicit);
            
            // NOTE(mal): Generate error for tags in positions such as "label:@tag {children}"
            MD_Node *fc = result.node->first_child;
            if(fc == result.node->last_child && !MD_NodeIsNil(fc->first_tag) && // NOTE(mal): One child. Tagged.
               fc->kind == MD_NodeKind_Label && fc->whole_string.size == 0)     // NOTE(mal): Unlabeled set
            {
                for(MD_EachNode(tag, fc->first_tag))
                {
                    _MD_Error(ctx, tag, MD_MessageKind_Error, "Invalid position for tag \"%.*s\"", MD_StringExpand(tag->string));
                }
            }
        }
        goto end_parse;
    }
    
    else if(MD_Parse_RequireKind(ctx, MD_TokenKind_BadCharacter, &token))
    {
        MD_String8List bytes = {0};
        for(int i_byte = 0; i_byte < token.outer_string.size; ++i_byte)
        {
            MD_PushStringToList(&bytes, MD_PushStringF("0x%02X", token.outer_string.str[i_byte]));
        }
        MD_String8 byte_string = MD_JoinStringList(bytes, MD_S8Lit(" "));
        _MD_TokenError(ctx, token, MD_MessageKind_Error, "Non-ASCII character \"%.*s\"", MD_StringExpand(byte_string));
        goto retry;
    }
    
    end_parse:;
    
    // NOTE(rjf): Parse comments after nodes.
    MD_String8 comment_after = MD_ZERO_STRUCT;
    {
        MD_Token comment_token = MD_ZERO_STRUCT;
        for(;;)
        {
            MD_Token token = MD_Parse_PeekSkipSome(ctx, 0);
            if(token.kind == MD_TokenKind_Comment)
            {
                comment_token = token;
                MD_Parse_Bump(ctx, token);
                break;
            }
            else if(token.kind == MD_TokenKind_Newline)
            {
                break;
            }
            else if(MD_TokenKindIsWhitespace(token.kind))
            {
                MD_Parse_Bump(ctx, token);
            }
            else
            {
                break;
            }
        }
        comment_after = comment_token.string;
        if(!_MD_CommentIsSyntacticallyCorrect(comment_token))
        {
            MD_String8 capped = MD_StringPrefix(comment_token.outer_string, MD_UNTERMINATED_TOKEN_LEN_CAP);
            _MD_TokenError(ctx, comment_token, MD_MessageKind_CatastrophicError,
                           "Unterminated comment \"%.*s\"", MD_StringExpand(capped));
        }
    }
    
    result.bytes_parsed = (MD_u64)(ctx->at - at_first);
    result.first_error = ctx_last_error ? ctx_last_error->next : 0;
    if(!MD_NodeIsNil(result.node))
    {
        result.node->first_tag = first_tag;
        result.node->last_tag = last_tag;
        for(MD_Node *tag = first_tag; !MD_NodeIsNil(tag); tag = tag->next)
        {
            tag->parent = result.node;
        }
        result.node->comment_before = comment_before;
        result.node->comment_after = comment_after;
    }
    return result;
}

MD_FUNCTION_IMPL MD_ParseResult
MD_ParseOneNode(MD_String8 filename, MD_String8 contents)
{
    MD_ParseCtx ctx = MD_Parse_InitializeCtx(filename, contents);
    return MD_ParseOneNodeFromCtx(&ctx);
}

MD_FUNCTION_IMPL MD_ParseResult
MD_ParseWholeString(MD_String8 filename, MD_String8 contents)
{
    MD_ParseResult result = MD_ZERO_STRUCT;
    MD_Node *root = MD_MakeNodeFromString(MD_NodeKind_File, filename, contents.str, contents.str, MD_PushStringF("`DD Parsed From \"%.*s\"`", MD_StringExpand(filename)));
    if(contents.size > 0)
    {
        // NOTE(mal): Parse the content of the file as the inside of a set
        MD_ParseCtx ctx = MD_Parse_InitializeCtx(filename, contents);
        MD_NodeFlags next_child_flags = 0;
        
        MD_Node *namespaces = _MD_MakeNodeFromString_Ctx(&ctx, MD_NodeKind_List, MD_S8Lit(""), ctx.at);
        MD_Node *default_namespace = _MD_MakeNodeFromString_Ctx(&ctx, MD_NodeKind_List, MD_S8Lit(""), ctx.at);
        MD_PushChild(namespaces, default_namespace);
        MD_Node *selected_namespace = default_namespace;
        MD_Map namespace_table = {0};
        MD_StringMap_Insert(&namespace_table, MD_MapCollisionRule_Overwrite, default_namespace->string, default_namespace);
        
        for(MD_u64 child_idx = 0;; child_idx += 1)
        {
            // NOTE(rjf): #-things (just namespaces right now, but can be used for other such
            // 'directives' in the future maybe)
            if(MD_Parse_Require(&ctx, MD_S8Lit("#"), MD_TokenKind_Symbol))
            {
                // NOTE(rjf): Namespaces
                if(MD_Parse_Require(&ctx, MD_S8Lit("namespace"), MD_TokenKind_Identifier))
                {
                    MD_Token token = MD_ZERO_STRUCT;
                    if(MD_Parse_RequireKind(&ctx, MD_TokenKind_Identifier, &token))
                    {
                        MD_MapSlot *existing_namespace_slot = MD_StringMap_Lookup(&namespace_table, token.string);
                        if(existing_namespace_slot == 0)
                        {
                            MD_Node *ns = _MD_MakeNodeFromString_Ctx(&ctx, MD_NodeKind_List, token.string,
                                                                     token.outer_string.str);
                            MD_StringMap_Insert(&namespace_table, MD_MapCollisionRule_Overwrite, token.string, ns);
                            existing_namespace_slot = MD_StringMap_Lookup(&namespace_table, token.string);
                            MD_PushChild(namespaces, ns);
                        }
                        selected_namespace = (MD_Node *)existing_namespace_slot->value;
                    }
                    else
                    {
                        selected_namespace = default_namespace;
                    }
                }
                // NOTE(rjf): Not a valid hash thing
                else
                {
                    MD_Token token = MD_Parse_PeekSkipSome(&ctx, 0);
                    _MD_TokenError(&ctx, token, MD_MessageKind_Error, "Invalid hash directive \"%.*s\"",
                                   MD_StringExpand(token.outer_string));
                }
            }
            
            MD_ParseResult parse = MD_ParseOneNodeFromCtx(&ctx);
            MD_Node *child = parse.node;
            child->flags |= next_child_flags;
            next_child_flags = 0;
            if(MD_NodeIsNil(child))
            {
                break;
            }
            else
            {
                MD_PushSibling(&root->first_child, &root->last_child, child);
                child->parent = root;
                MD_InsertToNamespace(selected_namespace, child);
            }
            
            if(MD_Parse_Require(&ctx, MD_S8Lit(","), MD_TokenKind_Symbol))
            {
                child->flags |= MD_NodeFlag_BeforeComma;
                next_child_flags |= MD_NodeFlag_AfterComma;
            }
            else if(MD_Parse_Require(&ctx, MD_S8Lit(";"), MD_TokenKind_Symbol))
            {
                child->flags |= MD_NodeFlag_BeforeSemicolon;
                next_child_flags |= MD_NodeFlag_AfterSemicolon;
            }
        }
        result.namespaces = namespaces;
        result.bytes_parsed = (MD_u64)(ctx.at - contents.str);
        result.first_error = ctx.first_error;
    }
    result.node = root;
    return result;
}

MD_FUNCTION_IMPL MD_ParseResult
MD_ParseWholeFile(MD_String8 filename)
{
    MD_String8 file_contents = MD_LoadEntireFile(filename);
    MD_ParseResult parse = MD_ParseWholeString(filename, file_contents);
    if(file_contents.str == 0)
    {
        MD_ParseCtx ctx = MD_Parse_InitializeCtx(filename, MD_S8Lit(""));
        _MD_Error(&ctx, parse.node, MD_MessageKind_CatastrophicError, "Could not read file \"%.*s\"",
                  MD_StringExpand(filename));
        parse.first_error = ctx.first_error;
    }
    return parse;
}

//~ Location Conversions

MD_PRIVATE_FUNCTION_IMPL MD_CodeLoc
MD_CodeLocFromFileOffset(MD_String8 filename, MD_u8 *base, MD_u8 *at)
{
    MD_CodeLoc loc;
    loc.filename = filename;
    loc.line = 1;
    loc.column = 1;
    for(MD_u64 i = 0; base+i < at && base[i]; i += 1)
    {
        if(base[i] == '\n')
        {
            loc.line += 1;
            loc.column = 1;
        }
        else
        {
            loc.column += 1;
        }
    }
    return loc;
}

MD_FUNCTION_IMPL MD_CodeLoc
MD_CodeLocFromNode(MD_Node *node)
{
    MD_CodeLoc loc = MD_CodeLocFromFileOffset(node->filename, node->file_contents, node->at);
    return loc;
}

//~ Tree/List Building

MD_FUNCTION_IMPL MD_b32
MD_NodeIsNil(MD_Node *node)
{
    return node == 0 || node == &_md_nil_node || node->kind == MD_NodeKind_Nil;
}

MD_FUNCTION_IMPL MD_Node *
MD_NilNode(void) { return &_md_nil_node; }

MD_FUNCTION_IMPL MD_Node *
MD_MakeNode(MD_NodeKind kind, MD_String8 string,
            MD_String8 whole_string, MD_String8 filename,
            MD_u8 *file_contents, MD_u8 *at)
{
    MD_Node *node = MD_PushArray(MD_Node, 1);
    node->kind = kind;
    node->string = string;
    node->whole_string = whole_string;
    node->next = node->prev = node->parent =
        node->first_child = node->last_child =
        node->first_tag = node->last_tag = node->ref_target = MD_NilNode();
    node->filename = filename;
    node->file_contents = file_contents;
    node->at = at;
    return node;
}

MD_FUNCTION_IMPL MD_Node *
MD_MakeNodeFromToken(MD_NodeKind kind, MD_String8 filename, MD_u8 *file, MD_u8 *at, MD_Token token)
{
    return MD_MakeNode(kind, token.string, token.outer_string, filename, file, at);
}

MD_FUNCTION_IMPL MD_Node *
MD_MakeNodeFromString(MD_NodeKind kind, MD_String8 filename, MD_u8 *file, MD_u8 *at, MD_String8 string)
{
    return MD_MakeNode(kind, string, string, filename, file, at);
}

MD_FUNCTION_IMPL MD_Node *
MD_MakeNodeReference(MD_Node *target)
{
    MD_Node *n = MD_MakeNodeFromString(MD_NodeKind_Reference, MD_S8Lit("`reference node`"), 0, 0, MD_S8Lit("`reference node`"));
    n->ref_target = target;
    return n;
}

MD_FUNCTION_IMPL void
MD_PushSibling(MD_Node **firstp, MD_Node **lastp, MD_Node *node)
{
    if(!MD_NodeIsNil(node))
    {
        MD_Node *first = *firstp;
        MD_Node *last = *lastp;
        if(MD_NodeIsNil(last))
        {
            first = last = node;
            node->next = node->prev = MD_NilNode();
        }
        else
        {
            last->next = node;
            node->next = MD_NilNode();
            node->prev = last;
            last = last->next;
        }
        *firstp = first;
        *lastp = last;
    }
}

MD_FUNCTION_IMPL void
MD_PushChild(MD_Node *parent, MD_Node *new_child)
{
    MD_PushSibling(&parent->first_child, &parent->last_child, new_child);
    new_child->parent = parent;
}

MD_FUNCTION_IMPL void
MD_PushTag(MD_Node *node, MD_Node *tag)
{
    MD_PushSibling(&node->first_tag, &node->last_tag, tag);
    tag->parent = node;
}

MD_FUNCTION_IMPL void
MD_InsertToNamespace(MD_Node *ns, MD_Node *node)
{
    MD_Node *ref = MD_MakeNode(MD_NodeKind_Reference, node->string,
                               node->whole_string, node->filename,
                               node->file_contents, node->at);
    ref->ref_target = node;
    MD_PushChild(ns, ref);
}

//~ Introspection Helpers

MD_FUNCTION_IMPL MD_Node *
MD_NodeFromString(MD_Node *first, MD_Node *last, MD_String8 string)
{
    MD_Node *result = MD_NilNode();
    for(MD_Node *node = first; !MD_NodeIsNil(node); node = node->next)
    {
        if(MD_StringMatch(string, node->string, 0))
        {
            result = node;
            break;
        }
    }
    return result;
}

MD_FUNCTION_IMPL MD_Node *
MD_NodeFromIndex(MD_Node *first, MD_Node *last, int n)
{
    MD_Node *result = MD_NilNode();
    if(n >= 0)
    {
        int idx = 0;
        for(MD_Node *node = first; !MD_NodeIsNil(node); node = node->next, idx += 1)
        {
            if(idx == n)
            {
                result = node;
                break;
            }
        }
    }
    return result;
}

MD_FUNCTION_IMPL int
MD_IndexFromNode(MD_Node *node)
{
    int idx = 0;
    if(node && !MD_NodeIsNil(node))
    {
        for(MD_Node *last = node->prev; !MD_NodeIsNil(last); last = last->prev, idx += 1);
    }
    return idx;
}

MD_FUNCTION_IMPL MD_Node *
MD_NextNodeSibling(MD_Node *last, MD_String8 string)
{
    MD_Node *result = MD_NilNode();
    if(last)
    {
        for(MD_Node *node = last->next; node; node = node->next)
        {
            if(MD_StringMatch(string, node->string, 0))
            {
                result = node;
                break;
            }
        }
    }
    return result;
}

MD_FUNCTION_IMPL MD_Node *
MD_ChildFromString(MD_Node *node, MD_String8 child_string)
{
    return MD_NodeFromString(node->first_child, node->last_child, child_string);
}

MD_FUNCTION_IMPL MD_Node *
MD_TagFromString(MD_Node *node, MD_String8 tag_string)
{
    return MD_NodeFromString(node->first_tag, node->last_tag, tag_string);
}

MD_FUNCTION_IMPL MD_Node *
MD_ChildFromIndex(MD_Node *node, int n)
{
    return MD_NodeFromIndex(node->first_child, node->last_child, n);
}

MD_FUNCTION_IMPL MD_Node *
MD_TagFromIndex(MD_Node *node, int n)
{
    return MD_NodeFromIndex(node->first_child, node->last_child, n);
}

MD_FUNCTION_IMPL MD_Node *
MD_TagArgFromIndex(MD_Node *node, MD_String8 tag_string, int n)
{
    MD_Node *tag = MD_TagFromString(node, tag_string);
    return MD_ChildFromIndex(tag, n);
}

MD_FUNCTION_IMPL MD_Node *
MD_TagArgFromString(MD_Node *node, MD_String8 tag_string, MD_String8 arg_string)
{
    MD_Node *tag = MD_TagFromString(node, tag_string);
    MD_Node *arg = MD_ChildFromString(tag, arg_string);
    return arg;
}

MD_FUNCTION_IMPL MD_b32
MD_NodeHasTag(MD_Node *node, MD_String8 tag_string)
{
    return !MD_NodeIsNil(MD_TagFromString(node, tag_string));
}

MD_FUNCTION_IMPL MD_i64
MD_ChildCountFromNode(MD_Node *node)
{
    MD_i64 result = 0;
    for(MD_EachNode(child, node->first_child))
    {
        result += 1;
    }
    return result;
}

MD_FUNCTION_IMPL MD_i64
MD_TagCountFromNode(MD_Node *node)
{
    MD_i64 result = 0;
    for(MD_EachNode(tag, node->first_tag))
    {
        result += 1;
    }
    return result;
}

MD_FUNCTION_IMPL MD_Node *
MD_Deref(MD_Node *node)
{
    MD_Node *result = node;
    while(result->kind == MD_NodeKind_Reference)
    {
        result = result->ref_target;
    }
    return result;
}

MD_FUNCTION MD_Node *
MD_SeekNodeWithFlags(MD_Node *start, MD_NodeFlags one_past_last_flags)
{
    MD_Node *result = start;
    for(MD_EachNode(it, start->next))
    {
        if(it->flags & one_past_last_flags)
        {
            break;
        }
        result = it;
    }
    return result;
}

//~ Error/Warning Helpers

MD_FUNCTION_IMPL void
MD_NodeMessage(MD_Node *node, MD_MessageKind kind, MD_String8 str)
{
    const char *kind_name = kind == MD_MessageKind_Error ? "error" : "warning";
    MD_CodeLoc loc = MD_CodeLocFromNode(node);
    fprintf(stderr, "%.*s:%i:%i: %s: %.*s\n",
            MD_StringExpand(loc.filename),
            loc.line, loc.column,
            kind_name,
            MD_StringExpand(str));
}

MD_FUNCTION_IMPL void
MD_NodeMessageF(MD_Node *node, MD_MessageKind kind, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    MD_NodeMessage(node, kind ,MD_PushStringFV(fmt, args));
    va_end(args);
}

//~ Tree Comparison/Verification

MD_FUNCTION_IMPL MD_b32
MD_NodeMatch(MD_Node *a, MD_Node *b, MD_MatchFlags flags)
{
    MD_b32 result = 0;
    if(a->kind == b->kind && MD_StringMatch(a->string, b->string, flags))
    {
        result = 1;
        if(a->kind != MD_NodeKind_Tag && (flags & MD_MatchFlag_Tags))
        {
            for(MD_Node *a_tag = a->first_tag, *b_tag = b->first_tag;
                !MD_NodeIsNil(a_tag) || !MD_NodeIsNil(b_tag);
                a_tag = a_tag->next, b_tag = b_tag->next)
            {
                if(MD_NodeMatch(a_tag, b_tag, flags))
                {
                    if(flags & MD_MatchFlag_TagArguments)
                    {
                        for(MD_Node *a_tag_arg = a_tag->first_child, *b_tag_arg = b_tag->first_child;
                            !MD_NodeIsNil(a_tag_arg) || !MD_NodeIsNil(b_tag_arg);
                            a_tag_arg = a_tag_arg->next, b_tag_arg = b_tag_arg->next)
                        {
                            if(!MD_NodeDeepMatch(a_tag_arg, b_tag_arg, flags))
                            {
                                result = 0;
                                goto end;
                            }
                        }
                    }
                }
                else
                {
                    result = 0;
                    goto end;
                }
            }
        }
    }
    end:;
    return result;
}

MD_FUNCTION_IMPL MD_b32
MD_NodeDeepMatch(MD_Node *a, MD_Node *b, MD_MatchFlags flags)
{
    MD_b32 result = MD_NodeMatch(a, b, flags);
    if(result)
    {
        for(MD_Node *a_child = a->first_child, *b_child = b->first_child;
            !MD_NodeIsNil(a_child) || !MD_NodeIsNil(b_child);
            a_child = a_child->next, b_child = b_child->next)
        {
            if(!MD_NodeDeepMatch(a_child, b_child, flags))
            {
                result = 0;
                goto end;
            }
        }
    }
    end:;
    return result;
}

//~ Generation

MD_FUNCTION_IMPL void
MD_OutputTree(FILE *file, MD_Node *node)
{
    for(MD_Node *tag = node->first_tag; !MD_NodeIsNil(tag); tag = tag->next)
    {
        fprintf(file, "@%.*s", MD_StringExpand(tag->string));
        if(!MD_NodeIsNil(tag->first_child))
        {
            fprintf(file, "(");
            for(MD_Node *child = tag->first_child; !MD_NodeIsNil(child); child = child->next)
            {
                MD_OutputTree(file, child);
                fprintf(file, ", ");
            }
            fprintf(file, ")\n");
        }
        else
        {
            fprintf(file, " ");
        }
    }
    
    fprintf(file, "%.*s", MD_StringExpand(node->whole_string));
    if(!MD_NodeIsNil(node->first_child))
    {
        fprintf(file, ":\n{\n");
        for(MD_Node *child = node->first_child; !MD_NodeIsNil(child); child = child->next)
        {
            MD_OutputTree(file, child);
            fprintf(file, ",\n");
        }
        fprintf(file, "}\n");
    }
    else
    {
        fprintf(file, " ");
    }
}

//~ Command Line Argument Helper

MD_FUNCTION MD_String8List
MD_StringListFromArgCV(int argument_count, char **arguments)
{
    MD_String8List options = MD_ZERO_STRUCT;
    for(int i = 1; i < argument_count; i += 1)
    {
        MD_PushStringToList(&options, MD_S8CString(arguments[i]));
    }
    return options;
}

MD_FUNCTION MD_CommandLine
MD_CommandLineFromOptions(MD_String8List options)
{
    MD_CommandLine cmdln = MD_ZERO_STRUCT;
    cmdln.arguments = options;
    
    for(MD_String8Node *n = options.first, *next = 0;
        n; n = next)
    {
        next = n->next;
        
        //- rjf: figure out whether or not this is an option by checking for `-` or `--`
        // from the beginning of the string
        MD_String8 option_name = MD_ZERO_STRUCT;
        if(MD_StringMatch(MD_StringPrefix(n->string, 2), MD_S8Lit("--"), 0))
        {
            option_name = MD_StringSkip(n->string, 2);
        }
        else if(MD_StringMatch(MD_StringPrefix(n->string, 1), MD_S8Lit("-"), 0))
        {
            option_name = MD_StringSkip(n->string, 1);
        }
        //- rjf: trim off anything after a `:` or `=`, use that as the first value string
        MD_String8 first_value = MD_ZERO_STRUCT;
        MD_b32 has_many_values = 0;
        if(option_name.size != 0)
        {
            MD_u64 colon_signifier_pos = MD_FindSubstring(option_name, MD_S8Lit(":"), 0, 0);
            MD_u64 equal_signifier_pos = MD_FindSubstring(option_name, MD_S8Lit("="), 0, 0);
            MD_u64 signifier_pos = colon_signifier_pos > equal_signifier_pos ? equal_signifier_pos : colon_signifier_pos;
            if(signifier_pos < option_name.size)
            {
                first_value = MD_StringSkip(option_name, signifier_pos+1);
                option_name = MD_StringPrefix(option_name, signifier_pos);
                if(MD_StringMatch(MD_StringSuffix(first_value, 1), MD_S8Lit(","), 0))
                {
                    has_many_values = 1;
                }
            }
        }
        
        //- rjf: gather arguments
        if(option_name.size != 0)
        {
            MD_String8List option_values = MD_ZERO_STRUCT;
            
            //- rjf: push first value
            if(first_value.size != 0)
            {
                MD_PushStringToList(&option_values, first_value);
            }
            
            //- rjf: scan next string values, add them to option values until we hit a lack
            // of a ',' between values
            if(has_many_values)
            {
                for(MD_String8Node *v = next; v; v = v->next, next = v)
                {
                    MD_String8 value_str = v->string;
                    MD_b32 next_has_arguments = MD_StringMatch(MD_StringSuffix(value_str, 1), MD_S8Lit(","), 0);
                    MD_b32 in_quotes = 0;
                    MD_u64 start = 0;
                    for(MD_u64 i = 0; i <= value_str.size; i += 1)
                    {
                        if(i == value_str.size || (value_str.str[i] == ',' && in_quotes == 0))
                        {
                            if(start != i)
                            {
                                MD_PushStringToList(&option_values, MD_StringSubstring(value_str, start, i));
                            }
                            start = i+1;
                        }
                        else if(value_str.str[i] == '"')
                        {
                            in_quotes = !in_quotes;
                        }
                    }
                    if(next_has_arguments == 0)
                    {
                        break;
                    }
                }
            }
            
            //- rjf: insert the fully parsed option
            {
                MD_CommandLineOption *opt = MD_PushArray(MD_CommandLineOption, 1);
                MD_MemoryZero(opt, sizeof(*opt));
                opt->name = option_name;
                opt->values = option_values;
                if(cmdln.last_option == 0)
                {
                    cmdln.first_option = cmdln.last_option = opt;
                }
                else
                {
                    cmdln.last_option->next = opt;
                    cmdln.last_option = cmdln.last_option->next;
                }
            }
        }
        
        //- rjf: this argument is not an option, push it to regular inputs list.
        else
        {
            MD_PushStringToList(&cmdln.inputs, n->string);
        }
    }
    
    return cmdln;
}

MD_FUNCTION MD_String8List
MD_CommandLineOptionValues(MD_CommandLine cmdln, MD_String8 name)
{
    MD_String8List values = MD_ZERO_STRUCT;
    for(MD_CommandLineOption *opt = cmdln.first_option; opt; opt = opt->next)
    {
        if(MD_StringMatch(opt->name, name, 0))
        {
            values = opt->values;
            break;
        }
    }
    return values;
}

MD_FUNCTION MD_b32
MD_CommandLineOptionPassed(MD_CommandLine cmdln, MD_String8 name)
{
    MD_b32 result = 0;
    for(MD_CommandLineOption *opt = cmdln.first_option; opt; opt = opt->next)
    {
        if(MD_StringMatch(opt->name, name, 0))
        {
            result = 1;
            break;
        }
    }
    return result;
}

MD_FUNCTION MD_i64
MD_CommandLineOptionI64(MD_CommandLine cmdln, MD_String8 name)
{
    MD_i64 v = 0;
    MD_String8List values = MD_CommandLineOptionValues(cmdln, name);
    MD_String8 value_str = MD_JoinStringList(values, MD_S8Lit(""));
    v = MD_I64FromString(value_str, 10);
    return v;
}

//~ File System

MD_FUNCTION_IMPL MD_String8
MD_LoadEntireFile(MD_String8 filename)
{
    MD_String8 file_contents = MD_ZERO_STRUCT;
    FILE *file = fopen((char*)MD_PushStringCopy(filename).str, "rb");
    if(file)
    {
        fseek(file, 0, SEEK_END);
        MD_u64 file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        file_contents.str = MD_PushArray(MD_u8, file_size+1);
        if(file_contents.str)
        {
            file_contents.size = file_size;
            fread(file_contents.str, 1, file_size, file);
        }
        fclose(file);
    }
    return file_contents;
}

MD_FUNCTION_IMPL MD_b32
MD_FileIterIncrement(MD_FileIter *it, MD_String8 path, MD_FileInfo *out_info)
{
#if !defined(MD_IMPL_FileIterIncrement)
    return(0);
#else
    return(MD_IMPL_FileIterIncrement(it, path, out_info));
#endif
}

/*
Copyright 2021 Dion Systems LLC

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
