#include "Json.h"

#include "Utils/Text/Formatting.h"
#include "Utils/Text/Num.h"

void JsonWriter::Null() {
    output += "null,";
}

void JsonWriter::Number(i64 num) {
    if (num < 0) { output += '-'; num = -num; }
    Text::NumberConversion::WriteU64Decimal(Text::StringWriter::WriteTo(output), num);
    Comma();
}

void JsonWriter::Number(u64 num) {
    Text::NumberConversion::WriteU64Decimal(Text::StringWriter::WriteTo(output), num);
    Comma();
}

void JsonWriter::StrLit(Str s) {
    s.WriteEscape(Text::StringWriter::WriteTo(output));
    Comma();
}

void JsonWriter::Key(Str key) {
    if (!key) return;
    // "[keyname]":
    ((output += '"') += key) += "\":";
}

void JsonWriter::Comma() {
    output += ',';
}

void JsonWriter::BeginArray(Str key) {
    Key(key); output += '[';
}

void JsonWriter::EndArray() {
    output.Last() = ']'; // kill the last comma
    Comma();
}

void JsonWriter::BeginObject(Str key) {
    Key(key); output += '{';
}

void JsonWriter::EndObject() {
    output.Last() = '}'; // kill the last comma
    Comma();
}

Str JsonWriter::Result() const {
    return output.Init();
}

JsonReader JsonReader::ForObject() {
    return { .kind = ReadingKind::OBJECT, .listeners = {} };
}

JsonReader JsonReader::ForArray(Listener listener) {
    return { .kind = ReadingKind::ARRAY, .listener = std::move(listener) };
}

JsonReader::~JsonReader() {
    switch (kind) {
        case ReadingKind::OBJECT: Memory::DestructAt(&listeners); break;
        case ReadingKind::ARRAY:  Memory::DestructAt(&listener);  break;
    }
}

void JsonReader::Accept(Str key, Any* dest, Decoder decoder) {
    Debug::QAssert$(kind == ReadingKind::OBJECT, "cant add listener with key for non-objects");
    listeners.Push({ key, { .dest = dest, .decoder = decoder } });
}

void JsonReader::AcceptStr(Str key, String* dest) {
    Accept(key, dest, &StrDecoder);
}

void JsonReader::Process(Str& input) const {
    if (kind == ReadingKind::OBJECT) ProcessObject(input);
    else if (kind == ReadingKind::ARRAY) ProcessArray(input);
}

void JsonReader::ProcessObject(Str& input) const {
    // { "key": [value], }
    Debug::QAssert$(input[0] == '{', "no opening braces");
    input.Advance(1);
    for (usize i = 0; i < input.Length();) {
        switch (input[i]) {
            case '}': input.Advance(i + 1); return;
            case '"': {
                const usize begin = ++i;
                for (; i < input.Length(); i++) {
                    if (input[i] == '"') break;
                }
                const Str currentKey = input.Substr(begin, i - begin);
                Debug::QAssert$(input[++i] == ':', "incomplete kvpair");

                const auto& listener = (*listeners.FindIf([&] (const auto& x) { return x[1_st] == currentKey; }))[2_nd];
                Str value = input.Substr(++i).TrimStart();
                listener.decoder(listener.dest, value);

                i = input.AsSpan().Unaddress(value.Data());
                if (input[i] == ',') ++i;
                break;
            }
            default: ++i; break;
        }
    }
}

void JsonReader::ProcessArray(Str& input) const {
    // [val1, val2]
    Debug::QAssert$(input[0] == '[', "no opening brackets");
    input.Advance(1);
    while (input = input.TrimStart(), input[0] != ']') {
        listener.decoder(listener.dest, input);
        if (input[0] == ',') input.Advance(1);
    }
}

void JsonReader::StrDecoder(String* out, Str& s) {
    Debug::QAssert$(s[0] == '"', "no opening quotes");
    s.Advance(1);
    while (s) {
        const char c = s[0];
        if (c == '\\') {
            char codepoints[4];
            u32 numRead, numWritten;
            if (!Chr::TryWriteEscape(s.Data(), s.Length(), numRead, codepoints, numWritten))
                return Debug::QError$("bad escape sequence {}", s);

            out->AppendStr(Str::Slice(codepoints, numWritten));
            s.Advance(numRead);
        } else if (c == '"') {
            s.Advance(1);
            return;
        } else {
            s.Advance(1);
            out->Append(c);
        }
    }
}

