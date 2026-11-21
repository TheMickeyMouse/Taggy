#pragma once
#include "Cache.h"
#include "Utils/Str.h"
#include "Utils/String.h"

using namespace Quasi;

class JsonWriter {
    String output;
public:
    JsonWriter() = default;

    void Null();
    void Number(i64 num);
    void Number(u64 num);
    void StrLit(Str s);
    template <class T> void Optional(const Option<T>& x) {
        if (x) Value(*x); else Null();
    }
    template <class T> void Value(const T& x) {
        if      constexpr (ConvTo<T, i64>) Number((i64)x);
        else if constexpr (ConvTo<T, u64>) Number((u64)x);
        else if constexpr (ConvTo<T, Str>) StrLit((Str)x);
        else if constexpr (requires () { { Option(x) } -> SameAs<T>; }) Optional(x);
        else { WriteJsonValue(*this, x); }
    }

    void Field(Str key, const auto& x) {
        Key(key); Value(x);
    }
    void Key(Str key);
    void Comma();

    void BeginArray(Str key = "");
    void EndArray();
    void BeginObject(Str key = "");
    void EndObject();
    Str Result() const;
};

class JsonReader {
public:
    using Any = void;
    template <class T> using DecoderFor = FnPtr<void, T*, Str&>;
    using Decoder = DecoderFor<Any>;
    struct Listener {
        void* dest;
        Decoder decoder;
    };

    enum class ReadingKind { OBJECT, ARRAY } kind;
    union {
        Vec<Tuple<Str, Listener>> listeners {}; // object
        Listener listener; // array
    };

    static JsonReader ForObject();
    static JsonReader ForArray(Listener listener);
    template <class T> static JsonReader ForArray(T* out, DecoderFor<T> decoder) {
        return ForArray(Listener { .dest = (Any*)out, .decoder = (Decoder)decoder });
    }

    ~JsonReader();

    void Accept(Str key, Any* dest, Decoder decoder);
    template <class T> void Accept(Str key, T* dest, DecoderFor<T> decoder) {
        Accept(key, (Any*)dest, (Decoder)decoder);
    }
    void AcceptStr(Str key, String* dest);
    template <Integer N> void AcceptNum(Str key, N* dest) {
        Accept(key, dest, &NumDecoder<N>);
    }
    template </* DecoderFor<T> */ auto Dec, class T> void AcceptVec(Str key, Vec<T>* dest) {
        Accept(key, (Any*)dest, (Decoder)ArrayDecoder<T, Dec>);
    }

    void Process(Str& input) const;
    void ProcessObject(Str& input) const;
    void ProcessArray(Str& input) const;

    template <Integer N> static void NumDecoder(N* out, Str& s) {
        const auto [i, val] = Text::ParsePartial<N>(s);
        if (!i) return Debug::QError$("bad number input {}", s);
        s.Advance(*i);
        *out = *val;
    }

    static void StrDecoder(String* out, Str& s);

    template <class T, DecoderFor<T> Dec> static void Opt(Option<T>* out, Str& s) {
        if (s.StartsWith("null")) { s.Advance(Q_STRLIT_LEN("null")); *out = nullptr; }
        else {
            T x {}; Dec(&x, s);
            *out = std::move(x);
        }
    }

    template <class T, DecoderFor<T> Dec> static void VecPushDecoder(Vec<T>* out, Str& s) {
        T x {}; Dec(&x, s);
        out->Push(std::move(x));
    }

    template <class T, DecoderFor<T> Dec> static void ArrayDecoder(Vec<T>* out, Str& s) {
        JsonReader r = ForArray(out, VecPushDecoder<T, Dec>);
        r.Process(s);
    }
};
