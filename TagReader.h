#pragma once
#include <fstream>

#include "Utils/Numeric.h"
#include "Utils/Option.h"
#include "nlohmann-json.h"
#include "Utils/CStr.h"
#include "Utils/Span.h"
#include "Utils/String.h"

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
        TRSO, TSIZ, TSRC, TSSE, TYER, TXXX, UFID, USER,
        USLT, WCOM, WCOP, WOAF, WOAR, WOAS, WORS, WPAY,
        WPUB, WXXX,

        CUSTOM_X = 1 << 25, CUSTOM_Y = 1 << 26, CUSTOM_Z = 1 << 27,
        CUSTOM = CUSTOM_X | CUSTOM_Y | CUSTOM_Z,
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
        return TAG_ID_NAME_LOOKUP[(u32)id];
    }
    const char* GetTagIDJsonName(TagID id);

    // unique file identifier
    struct UFID {
        // spec section 4.1
        CStr owner;
        Bytes identifier;
    };

    struct Tag {
        TagID id;
        u32 size;
    };

    struct Metadata {
        // first 8 bits is major, other is minor
        u16 version;
        u32 size;
        Vec<Tag> tags;
    };
}

class TagReader {
    std::ifstream file;
public:
    TagReader(const char* filename);
    Option<ID3v1> ReadV1();

    bool ReadV2Header(Out<ID3v2::Metadata&> meta);
    bool ReadV2FrameHeader(Out<ID3v2::Tag&> tag);

    ArrayBox<char> ReadTagPayload(u32 size);
    bool ReadV2UFID(u32 size, ID3v2::UFID& ufid);

    bool ReadTextWithEncoding(bool isUtf16, BytesMut string, String& result) const;
    OptionUsize FindNullTerminator(bool isUtf16, Bytes string) const;
    bool ReadV2TextField    (BytesMut data, String& textOut) const;
    bool ReadV2UserTextField(BytesMut data, String& descOut, String& valOut) const;
    bool ReadV2URLField     (BytesMut data, String& urlOut) const;
    bool ReadV2UseURLField  (BytesMut data, String& descOut, String& urlOut) const;

    Option<ID3v2::Metadata> ReadV2();
};