#pragma once
#include <fstream>

#include "Utils/Numeric.h"
#include "Utils/Option.h"
#include "nlohmann-json.h"
#include "Utils/CStr.h"
#include "Utils/Span.h"
#include "Utils/String.h"
#include "Utils/Variant.h"

using namespace Quasi;

struct alignas(void*) ID3v1 {
    // according to https://wiki.hydrogenaudio.org/index.php?title=ID3v1
    // | Song title  | 30 characters |
    // | Artist      | 30 characters |
    // | Album       | 30 characters |
    // | Year        | 4 characters  |
    // | Comment     | 30 characters |
    // | Genre       | 1 byte        |
    // total: 128 bytes
    char _header[3]; // initial padding; 3 bytes; always TAG so not used.
    char title[30];
    char artist[30];
    char album[30];
    char year[4];
    char comment[30];
    u8 genre;
    static char* GENRE_LIST[80];
};
static_assert(sizeof(ID3v1) == 128 && "ID3v1 must be 128 bytes (according to spec)!");

namespace ID3v2 {
    struct Header {
        // according to https://id3.org/id3v2.3.0 section 3.1
        // File Identifier	"ID3"
        // Version	        0x03 0x00 (version 2.**3**.**0** is most widely supported)
        // Flags	        0babc00000
        // Size	            4 * 0b0xxxxxxx
        char _fileIdentifier[3]; // initial padding;
        u8 versionMajor, versionMinor;
        u8 flags;
        u8 size[4]; // base 128 encoded integer; (can't use u32; will cause padding)
        // total: 10 bytes
        enum : u8 {
            UNSYNC          = 1 << 7, // see section 5
            EXTENDED_HEADER = 1 << 6, // see section 3.2
            EXPERIMENTAL    = 1 << 5,
            SHOULD_BE_ZEROS = (1 << 5) - 1,
        };
    };
    static_assert(sizeof(Header) == 10);


    struct FrameHeader {
        // spec section 3.3
        char id[4];
        u8 size[4]; // remove alignment issues
        u8 wFlags; // relating to the *writing* of this frame
        u8 rFlags; // relating to the *reading* of this frame
        enum : u8 {
            // see section 3.3.1
            // on = "if this frame is unknown (not in the system), it should be removed if a program changes the ID3v2 tags in any way shape or form"
            W_REMOVE_AFTER_TAGS_CHANGE = 1 << 7,
            // on = "if this frame is unknown (not in the system), it should be removed if a program changes the audio contents in any way shape or form"
            W_REMOVE_AFTER_FILE_CHANGE = 1 << 6,
            // on = "you *might* break something if you change this frame, but if you *do* change it improperly then you should clear this flag"
            W_READ_ONLY                = 1 << 5,

            // on = "this data is compressed with zlib & 4 bytes after this header is the compressed size of the contents"
            R_COMPRESSED = 1 << 7,
            // on = "this data is encrypted and the method is specified 1 byte after this header"
            R_ENCRYPTED  = 1 << 6,
            // on = "this data is belongs in a group and a groupID is specified 1 byte after this header"
            R_GROUPED    = 1 << 5,

            SHOULD_BE_ZEROS          = (1 << 5) - 1,
        };
    };
    static_assert(sizeof(FrameHeader) == 10);


    enum class TagID : u32 {
        NONE = 0,
        AENC, APIC, COMM, COMR, ENCR, EQUA, ETCO, GEOB,
        GRID, IPLS, LINK, MCDI, MLLT, OWNE, PRIV, PCNT,
        POPM, POSS, RBUF, RVAD, RVRB, SYLT, SYTC, TALB,
        TBPM, TCOM, TCON, TCOP, TDAT, TDLY, TENC, TEXT,
        TFLT, TIME, TIT1, TIT2, TIT3, TKEY, TLAN, TLEN,
        TMED, TOAL, TOFN, TOLY, TOPE, TORY, TOWN, TPE1,
        TPE2, TPE3, TPE4, TPOS, TPUB, TRCK, TRDA, TRSN,
        TRSO, TSIZ, TSRC, TSSE, TXXX, TYER, UFID, USER,
        USLT, WCOM, WCOP, WOAF, WOAR, WOAS, WORS, WPAY,
        WPUB, WXXX, LAST_NORMAL,
    };
    static constexpr u32 TAG_ID_NAMES_INDEX[] = {
        "AENC"_u32, "APIC"_u32, "COMM"_u32, "COMR"_u32, "ENCR"_u32, "EQUA"_u32, "ETCO"_u32, "GEOB"_u32,
        "GRID"_u32, "IPLS"_u32, "LINK"_u32, "MCDI"_u32, "MLLT"_u32, "OWNE"_u32, "PRIV"_u32, "PCNT"_u32,
        "POPM"_u32, "POSS"_u32, "RBUF"_u32, "RVAD"_u32, "RVRB"_u32, "SYLT"_u32, "SYTC"_u32, "TALB"_u32,
        "TBPM"_u32, "TCOM"_u32, "TCON"_u32, "TCOP"_u32, "TDAT"_u32, "TDLY"_u32, "TENC"_u32, "TEXT"_u32,
        "TFLT"_u32, "TIME"_u32, "TIT1"_u32, "TIT2"_u32, "TIT3"_u32, "TKEY"_u32, "TLAN"_u32, "TLEN"_u32,
        "TMED"_u32, "TOAL"_u32, "TOFN"_u32, "TOLY"_u32, "TOPE"_u32, "TORY"_u32, "TOWN"_u32, "TPE1"_u32,
        "TPE2"_u32, "TPE3"_u32, "TPE4"_u32, "TPOS"_u32, "TPUB"_u32, "TRCK"_u32, "TRDA"_u32, "TRSN"_u32,
        "TRSO"_u32, "TSIZ"_u32, "TSRC"_u32, "TSSE"_u32, "TXXX"_u32, "TYER"_u32, "UFID"_u32, "USER"_u32,
        "USLT"_u32, "WCOM"_u32, "WCOP"_u32, "WOAF"_u32, "WOAR"_u32, "WOAS"_u32, "WORS"_u32, "WPAY"_u32,
        "WPUB"_u32, "WXXX"_u32,
    };
    static constexpr char (TAG_ID_NAME_LOOKUP[])[5] = {
        "AENC", "APIC", "COMM", "COMR", "ENCR", "EQUA", "ETCO", "GEOB",
        "GRID", "IPLS", "LINK", "MCDI", "MLLT", "OWNE", "PRIV", "PCNT",
        "POPM", "POSS", "RBUF", "RVAD", "RVRB", "SYLT", "SYTC", "TALB",
        "TBPM", "TCOM", "TCON", "TCOP", "TDAT", "TDLY", "TENC", "TEXT",
        "TFLT", "TIME", "TIT1", "TIT2", "TIT3", "TKEY", "TLAN", "TLEN",
        "TMED", "TOAL", "TOFN", "TOLY", "TOPE", "TORY", "TOWN", "TPE1",
        "TPE2", "TPE3", "TPE4", "TPOS", "TPUB", "TRCK", "TRDA", "TRSN",
        "TRSO", "TSIZ", "TSRC", "TSSE", "TXXX", "TYER", "UFID", "USER",
        "USLT", "WCOM", "WCOP", "WOAF", "WOAR", "WOAS", "WORS", "WPAY",
        "WPUB", "WXXX",
    };
    static constexpr const char* GetTagIDCode(TagID id) {
        return TAG_ID_NAME_LOOKUP[(u32)id - 1];
    }

    enum class TagFormat {
        OTHER = 0, // means it depends on the tag id itself
        TEXT,  // standard text format
        YEAR,  // year format YYYY (4 chars)
        NCHAR, // numeric string
    };
    struct TagRules {
        bool allowsMultiple, supported, deprecatedInV4;
        u8 supportedMajorVersion;
        const char* jsonName, *properName;
        TagFormat format;
    };
    const TagRules& GetTagRules(TagID id);

    struct None {
        void Print() const;
        bool Write(Vec<byte>&) const;
    };

    // unique file identifier
    struct UFID {
        // spec section 4.1
        CStr owner;
        Bytes identifier;

        void Print() const;
        bool Read(BytesMut data);
        bool Write(Vec<byte>& data) const;
    };

    struct TextField {
        String value;
        void Print() const;
        bool Read(BytesMut data);
        bool Write(Vec<byte>& data) const;

        static TextField FromList(Span<const Str> vals);
    };

    struct CustomTextField {
        String desc, value;
        void Print() const;
        bool Read(BytesMut data);
        bool Write(Vec<byte>& data) const;
    };

    struct YearField {
        u32 year;
        void Print() const;
        bool Read(BytesMut data);
        bool Write(Vec<byte>& data) const;
    };
    struct DateField {
        u32 month, day;
        void Print() const;
        bool Read(BytesMut data);
        bool Write(Vec<byte>& data) const;
    };
    struct TimeField {
        u32 hour, min;
        void Print() const;
        bool Read(BytesMut data);
        bool Write(Vec<byte>& data) const;
    };

    struct NumberField {
        u64 value;
        void Print() const;
        bool Read(BytesMut data);
        bool Write(Vec<byte>& data) const;
    };

    struct PictureField {
        // filetype = 0: png, 1: jpeg
        u8 pictureType, fileType;
        String desc;
        ArrayBox<byte> pictureData;

        void Print() const;
        bool Read(BytesMut data);
        bool Write(Vec<byte>& data) const;
    };

    // a 3 byte value, where each byte is a character (lowercase)
    enum Language : u32 {
        ENGLISH = "eng"_u32,
        JAPANESE = "jpn"_u32,
        PORTUGESE = "por"_u32
    };
    Array<char, 3> GetLangCode(Language lang);

    struct UnsyncedLyricsField {
        Language lang;
        String desc, lyrics;

        void Print() const;
        bool Read(BytesMut data);
        bool Write(Vec<byte>& data) const;
    };

    enum class SyncedContentType {
        OTHER,         // is other
        LYRICS,        // is lyrics
        TRANSCRIPTION, // is text transcription
        PART_NAME,     // is movement/part name (e.g. "Adagio")
        EVENTS,        // is events (e.g. "Don Quijote enters the stage")
        CHORDS,        // is chord (e.g. "Bb F Fsus")
        POP_UP_INFO,   // is trivia/'pop up' information
    };
    enum class TimeStampUnit {
        FRAMES,
        MILLISECONDS,
    };

    struct SyncedLyrics {
        // FORMAT: (always in native endian)
        // /--- 4 ---\ /--- 4 ---\ /--- * --- ...
        //  TIMESTAMP   LENGTH      LYRICS...
        Vec<u8> raw;

        // iterator pattern; yields timestamp + lyrics
        static Tuple<u32, Str> Next(Bytes& data);

        void Print() const;
        bool Read(bool isUtf16, BytesMut data);
        bool Write(bool isUtf16, Vec<byte>& data) const;
        bool NeedsUtf16() const;
        void Add(u32 timestamp, Str lyrics);
    };

    struct SyncedLyricsField {
        Language lang;
        SyncedContentType type;
        TimeStampUnit timeUnit;
        String desc;
        SyncedLyrics lyrics;

        void Print() const;
        bool Read(BytesMut data);
        bool Write(Vec<byte>& data) const;
    };

    using Field = Variant<
        None,
        UFID,
        TextField,
        CustomTextField,
        YearField,
        DateField,
        TimeField,
        NumberField,
        PictureField,
        UnsyncedLyricsField,
        SyncedLyricsField
    >;
    void PrintField(const Field& field);
    bool WriteField(const Field& field, Vec<byte>& data);

    struct Tag {
        TagID id;
        u32 size = 0;
        Tag(TagID id = TagID::NONE, u32 size = 0) : id(id), size(size) {}
    };

    enum MpegVersion : u8 {
        V1 = 1, V2, V2_5
    };

    struct Metadata {
        // first 8 bits is major, other is minor
        u16 tagVersion;
        MpegVersion mpgVersion;
        u8 layerVersion;
        u16 samplingRate, bitrateKbps;
        u8 channels;

        u32 size;
        Vec<Tag> tags;
        Vec<Field> tagFields;
    };

    struct TagFieldPair {
        Tag tag;
        Field field;
    };

    namespace Tags {
        TagFieldPair Artist(Str artists);
        TagFieldPair Album(Str album);
        TagFieldPair Title(Str title);
        TagFieldPair Disc(Str num);
        TagFieldPair AlbumArtist(Str artist);
        TagFieldPair UsLyrics(String unsynced);
        TagFieldPair SyLyrics(Str synced);
        TagFieldPair Cover(Vec<byte> coverData);
        TagFieldPair Explicit();
    }
}

class TagReader {
public:
    std::ifstream file;

    TagReader(const char* filename);
    Option<ID3v1> ReadV1();

    bool ReadV2Header(Out<ID3v2::Metadata&> meta);
    bool ReadV2TagHeader(Out<ID3v2::Tag&> tag);
    bool ReadV2TagData(const ID3v2::Tag& t, Out<ID3v2::Field&> payload);
    static bool ReadMpegData(std::istream& file, ID3v2::Metadata& meta, Out<OptRef<u32>> firstPosition = nullptr);

    ArrayBox<byte> ReadTagPayload(u32 size);
    static bool ReadTextWithEncoding(bool isUtf16, BytesMut string, String& result, u32& bytesRead);
    static bool ReadNumeric4Char(BytesMut string, char (&result) [4]);
    static bool ReadNumericString(BytesMut string, u64& result);

    Option<ID3v2::Metadata> ReadV2();
};

class TagWriter {
public:
    std::ifstream input;
    std::ofstream file;
    TagWriter(const char* source, const char* filename);

    bool WriteV2(Span<const ID3v2::TagFieldPair> tags);
    void WriteV2Header(u32 size);
    static bool WriteV2TagHeader(byte* data, const ID3v2::Tag& tag); // always writes 10 bytes
    static void WriteMpegFrames(std::istream& input, std::ostream& output);

    static bool NeedsUtf16(Str text);
    static bool WriteUtfString(bool useUtf16, Vec<byte>& data, Str text, bool includeNullTerm = true);
    static bool WriteLatinString(Vec<byte>& data, Str text, bool includeNullTerm = true);
    static bool WriteUtf16String(Vec<byte>& data, Str text, bool includeNullTerm = true);

};