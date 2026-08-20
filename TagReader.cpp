#include "TagReader.h"

#include "Utils/Debug/Logger.h"
#include "Utils/Algorithm.h"
#include "Utils/Text/UTF.h"

namespace ID3v2 {
#pragma region Tag Rules
    constexpr bool YES = true, NO = false;
    constexpr TagRules TAG_RULES[] = {
        // X = will not add, U = possibly
        // [allowsMultiple] [supported] [deprecated]
        { YES, NO,  NO,  "audioEncryption",  "Audio Encryption",  TagFormat::OTHER }, // X AENC: the encryption method of the audio ???
        { YES, NO,  NO,  "cover",            "Cover",             TagFormat::OTHER }, //   APIC?: cover pictures, can also include other pictures. see section 4.15
        { YES, NO,  NO,  "comment",          "Comment",           TagFormat::OTHER }, //   COMM: comments, format: [ENC (1b)] [LANG (3b)] [CONTENT DESCRIPTOR (?b)] \0 [TEXT (?b) W/ NEWLINES]
        { NO,  NO,  NO,  "purchaseInfo",     "Purchase Info",     TagFormat::OTHER }, // X COMR: about the purchase
        { YES, NO,  NO,  "__encryption",     "_Encryption",       TagFormat::OTHER }, // X ENCR: about the encryption info of other fields
        { NO,  NO,  YES, "equalization",     "Equalization",      TagFormat::OTHER }, // X EQUA: equalize each frequency. complicated, see section 4.13
        { NO,  NO,  NO,  "events",           "Events",            TagFormat::OTHER }, // U ETCO: list of events like opening, outro... see section 4.6
        { YES, NO,  NO,  "extraData",        "Extra Data",        TagFormat::OTHER }, // X GEOB: arbitrary extra files attached to the mp3
        { YES, NO,  NO,  "__grouping",       "_Grouping",         TagFormat::OTHER }, // X GRID: grouping info
        { NO,  NO,  YES, "involvedPeople",   "Involved People",   TagFormat::OTHER }, //   IPLS: everybody involved, format: person - involvment\0person2 - involvement\0...
        { YES, NO,  NO,  "__link",           "_Link",             TagFormat::OTHER }, // X LINK: 'if you see this, grab a copy of a specific field from a different file and paste it here'
        { NO,  NO,  NO,  "cd",               "CD",                TagFormat::OTHER }, // X MCDI: cd, so it can be identified in a database like CDDB. binary dump, see section 4.5
        { NO,  NO,  NO,  "_mpgLocationLT",   "_MLLT",             TagFormat::OTHER }, // X MLLT: ??? section 4.7
        { NO,  NO,  NO,  "ownershipProof",   "Ownership Proof",   TagFormat::OTHER }, // X OWNE: proof of ownership. may include transaction info, payment amount...
        { NO,  NO,  NO,  "playCount",        "Play Count",        TagFormat::OTHER }, //   PCNT: play count, >=5 bytes
        { NO,  NO,  NO,  "rating",           "Rating",            TagFormat::OTHER }, // U POPM: 'popularity' for a song. see section 4.18
        { NO,  NO,  NO,  "playPosition",     "Play Position",     TagFormat::OTHER }, // X POSS: what offset the song should be played. intended for unfinished listens and may want to listen again
        { YES, NO,  NO,  "privateData",      "Private Data",      TagFormat::OTHER }, // X PRIV: misc. info...
        { NO,  NO,  NO,  "preferredBufSize", "Pref. Buffer Size", TagFormat::OTHER }, // X RBUF: recommended buffer size when streaming
        { NO,  NO,  YES, "volumeAdjustment", "Volume Adjustment", TagFormat::OTHER }, // X RVAD: volume adjustment. complicated, see section 4.12
        { NO,  NO,  NO,  "reverb",           "Reverb",            TagFormat::OTHER }, // X RVRB: reverb amount. complicated, see section 4.14
        { YES, NO,  NO,  "syncedLyrics",     "Synced Lyrics",     TagFormat::OTHER }, //   SYLT: synced lyrics
        { NO,  NO,  NO,  "tempos",           "Tempos",            TagFormat::OTHER }, // U SYTC: list of (bpm, timestamp) pairs, section 4.8
        { NO,  YES, NO,  "album",            "Album",             TagFormat::TEXT  }, //   TALB: (song source, could be album/movie/show),
        { NO,  YES, NO,  "bpm",              "BPM",               TagFormat::NCHAR }, //   TBPM: (bpm, number format)
        { NO,  YES, NO,  "composer",         "Composer",          TagFormat::TEXT  }, //   TCOM: (composers, list format, separated by a '/')
        { NO,  YES, NO,  "genre",            "Genre",             TagFormat::TEXT  }, // U TCON: (see section 4.2.1),
        { NO,  YES, NO,  "copyrightYear",    "Copyright Year",    TagFormat::TEXT  }, //   TCOP: (should be a year + a space + ... (>= 5 chars). when displayed, should be "Copyright © [this value]")
        { NO,  YES, YES, "date",             "Date",              TagFormat::OTHER }, //   TDAT: (format: DDMM, always 4 chars)
        { NO,  YES, NO,  "playlistDelay",    "Playlist Delay",    TagFormat::NCHAR }, // X TDLY: (amount of delay (milliseconds) when playing between songs, number format)
        { NO,  YES, NO,  "encodedBy",        "Encoded By",        TagFormat::TEXT  }, // X TENC: (encoding origin, may be person/org. may contain copyright)
        { NO,  YES, NO,  "lyricist",         "Lyricist",          TagFormat::TEXT  }, //   TEXT: (the lyricists of the song; list format)
        { NO,  YES, NO,  "fileType",         "File Type",         TagFormat::TEXT  }, // U TFLT: (can be: MPG (default)|/1|/2|/3|/2.4|/AAC|VQF|PCM)
        { NO,  YES, YES, "time",             "Time",              TagFormat::OTHER }, //   TIME: (foramt: HHMM, always 4 chars)
        { NO,  YES, NO,  "category",         "Category",          TagFormat::TEXT  }, // U TIT1: (song category, similar to genre but more broad)
        { NO,  YES, NO,  "name",             "Name",              TagFormat::TEXT  }, //   TIT2: (name of the song)
        { NO,  YES, NO,  "description",      "Description",       TagFormat::TEXT  }, //   TIT3: (description)
        { NO,  YES, NO,  "firstKey",         "First Key",         TagFormat::TEXT  }, // X TKEY: (first key in the music, can be ([A-G]?[b#]?[m])|o)
        { NO,  YES, NO,  "language",         "Language",          TagFormat::TEXT  }, //   TLAN: (language code, 3 chars ISO-639-2)
        { NO,  YES, NO,  "duration",         "Duration",          TagFormat::NCHAR }, //   TLEN: (duration of song (milliseconds), number format)
        { NO,  YES, NO,  "mediaType",        "Media Type",        TagFormat::TEXT  }, // U TMED: (song origin ex: (DIG) for digital. see section 4.2.1)
        { NO,  YES, NO,  "originalAlbum",    "Original Album",    TagFormat::TEXT  }, // X TOAL: (original song source, if it was readapted/remix/cover of a different album)
        { NO,  YES, NO,  "originalName",     "Original Name",     TagFormat::TEXT  }, // X TOFN: (original filename, if the actual filename had to be changed to abide by restrictions)
        { NO,  YES, NO,  "originalLyricist", "Original Lyricist", TagFormat::TEXT  }, // X TOLY: (original lyricist, for adapted songs, list format)
        { NO,  YES, NO,  "originalArtists",  "Original Artists",  TagFormat::TEXT  }, // X TOPE: (original artists/performers, list format)
        { NO,  YES, YES, "originalYear",     "Original Year",     TagFormat::TEXT  }, // X TORY: (original release year, 4 chars)
        { NO,  YES, NO,  "owner",            "Owner",             TagFormat::TEXT  }, // X TOWN: (file owner/liscensee)
        { NO,  YES, NO,  "artists",          "Artists",           TagFormat::TEXT  }, //   TPE1: (artists/performers, list format)
        { NO,  YES, NO,  "band",             "Band",              TagFormat::TEXT  }, //   TPE2: (additional info for artists)
        { NO,  YES, NO,  "conductor",        "Conductor",         TagFormat::TEXT  }, // U TPE3: (conductor)
        { NO,  YES, NO,  "remixedBy",        "Remixed By",        TagFormat::TEXT  }, // X TPE4: (info about remix creator)
        { NO,  YES, NO,  "part",             "Part",              TagFormat::NCHAR }, // U TPOS: (which part the audio came from, if the album contains many mediums
        { NO,  YES, NO,  "publisher",        "Publisher",         TagFormat::TEXT  }, // X TPUB: (publisher)
        { NO,  YES, NO,  "trackNumber",      "Track Number",      TagFormat::NCHAR }, // U TRCK: (#, same format as TPOS)
        { NO,  YES, NO,  "recordedDate",     "Recorded Date",     TagFormat::TEXT  }, // X TRDA: (complement to other date info, any text no specific format, ex: 4th-7th June)
        { NO,  YES, NO,  "radioName",        "Radio Name",        TagFormat::TEXT  }, // X TRSN: (name of radio station which was streamed from)
        { NO,  YES, NO,  "radioOwner",       "Radio Owner",       TagFormat::TEXT  }, // X TRSO: (radio station owner)
        { NO,  YES, YES, "filesize",         "Filesize",          TagFormat::NCHAR }, // X TSIZ: (file size excluding ID3v2, in bytes)
        { NO,  YES, NO,  "irscCode",         "ISRC Code",         TagFormat::TEXT  }, // X TSRC: (international standard recording code, 12 chars)
        { NO,  YES, NO,  "encoderSettings",  "Encoder Settings",  TagFormat::TEXT  }, // X TSSE: (settings for audio encoder used)
        { NO,  YES, NO,  "extraData",        "Extra Data",        TagFormat::OTHER }, //   TXXX: (user defined)
        { NO,  YES, YES, "year",             "Year",              TagFormat::YEAR  }, //   TYER: (year, 4 chars)
        { YES, YES, NO,  "ufid",             "UFID",              TagFormat::OTHER }, //   UFID: unique file identifier, binary
        { NO,  NO,  NO,  "termsOfUse",       "Terms Of Use",      TagFormat::OTHER }, // X USER: terms of use
        { YES, NO,  NO,  "unsyncedLyrics",   "Unsynced Lyrics",   TagFormat::OTHER }, //   USLT: unsynced lyrics
        { YES, NO,  NO,  "comercialInfo",    "Comercial Info",    TagFormat::OTHER }, // X WCOM: (where you can buy the album, may be multiple)
        { NO,  NO,  NO,  "copyright",        "Copyright",         TagFormat::OTHER }, // X WCOP: (terms of use & ownership)
        { NO,  NO,  NO,  "webpage",          "Webpage",           TagFormat::OTHER }, // X WOAF: (official audio webpage)
        { YES, NO,  NO,  "artistWebpage",    "Artist Webpage",    TagFormat::OTHER }, // X WOAR: (official artist webpage)
        { NO,  NO,  NO,  "sourceWebpage",    "Source Webpage",    TagFormat::OTHER }, // X WOAS: (official source/album/movie/show webpage)
        { NO,  NO,  NO,  "radioWebpage",     "Radio Webpage",     TagFormat::OTHER }, // X WORS: (official radio station webpage)
        { NO,  NO,  NO,  "payment",          "Payment",           TagFormat::OTHER }, // X WPAY: (payment handling webpage)
        { NO,  NO,  NO,  "publisherWebpage", "Publisher Webpage", TagFormat::OTHER }, // X WPUB: (publisher webpage)
    };

    const TagRules& GetTagRules(TagID id) {
        return TAG_RULES[(u32)id - (u32)TagID::AENC];
    }
#pragma endregion

    // ADDED:
    // * TALB : (song source, could be album/movie/show),
    // * TBPM : (bpm, number format)
    // * TCOM : (composers, list format, separated by a '/')
    // * TCOP : (should be a year + a space + ... (>= 5 chars). when displayed, should be "Copyright © [this value]")
    // * TDAT : (format: DDMM, always 4 chars)
    // * TEXT : (the lyricists of the song; list format)
    // * TIME : (foramt: HHMM, always 4 chars)
    // * TIT2 : (name of the song)
    // * TIT3 : (description)
    // * TLAN : (language code, 3 chars ISO-639-2)
    // * TLEN : (duration of song (milliseconds), number format)
    // * TPE1 : (artists/performers, list format)
    // * TPE2 : (additional info for artists)
    // * TXXX : (user defined)
    // * TYER : (year, 4 chars)
    //   UFID*: unique file identifier, binary
    // TODO TO ADD:
    //     EASY:
    //         PCNT : play count, >=4 bytes
    //     TRICKY:
    //         APIC*?: cover pictures, can also include other pictures. see section 4.15
    //         COMM*: comments, format: [ENC (1b)] [LANG (3b)] [CONTENT DESCRIPTOR (?b)] \0 [TEXT (?b) W/ NEWLINES]
    //         IPLS : everybody involved, format: person - involvment\0person2 - involvement\0...
    //         SYLT*: synced lyrics, format: [ENC (1b)] [LANG (3b)] [TIMEFMT (1b)] [CONTENTTYPE (1b)] [DESC (?b)] \0 [TEXT_CHUNK\0TIME...]
    //         USLT*: unsynced lyrics, format: [ENCODING (1b)] [LANGUAGE (3b)] [CONTENT DESCRIPTOR (?b)] \0 [LYCRIS W/ NEWLINES]
    // TODO: MIGHT ADD:
    //     ETCO : list of events like opening, outro... see section 4.6
    //     POPM : 'popularity' for a song. see section 4.18
    //     SYTC : list of (bpm, timestamp) pairs, section 4.8
    //     TCON : (see section 4.2.1),
    //     TFLT : (can be: MPG (default)|/1|/2|/3|/2.4|/AAC|VQF|PCM)
    //     TIT1 : (song category, similar to genre but more broad)
    //     TMED : (song origin ex: (DIG) for digital. see section 4.2.1)
    //     TPE3 : (conductor)
    //     TPOS : (which part the audio came from, if the album contains many mediums. format: num/total (latter and slash is option))
    //     TRCK : (#, same format as TPOS)

#pragma region Reading & Parsing Tag Data
    void None::Print() const {
        Text::Print("null");
    }

    void UFID::Print() const {
        Text::Print("[owner = {}, identifier = ...binary data...]", owner);
    }

    bool UFID::Read(BytesMut data) {
        // UFID:
        // Owner identifier    <text string> 0x00
        // Identifier    <up to 64 bytes binary data>
        owner = { (const char*)data.Data() };
        identifier = Bytes::Slice((const u8*)owner.DataEndWithNull(), data.Length() - owner.LengthWithNull());
        return true;
    }

    void TextField::Print() const {
        Text::Print(value);
    }

    bool TextField::Read(BytesMut data) {
        // Text encoding    $xx
        // Information    <text string according to encoding>
        const bool isUTF16 = data[0];
        u32 _bytesRead; // dummy
        return TagReader::ReadTextWithEncoding(isUTF16, data.Skip(1), value, _bytesRead);
    }

    void CustomTextField::Print() const {
        Text::Print("[{}: {}]", desc, value);
    }

    bool CustomTextField::Read(BytesMut data) {
        // <Header for 'User defined text information frame', ID: "TXXX">
        // Text encoding    $xx
        // Description    <text string according to encoding> $00 (00)
        // Value    <text string according to encoding>
        const bool isUTF16 = data[0];

        u32 bytesRead = 0, _;
        return TagReader::ReadTextWithEncoding(isUTF16, data.Skip(1), desc, bytesRead)
            && TagReader::ReadTextWithEncoding(isUTF16, data.Subspan(bytesRead + 1), value, _);
    }

    void YearField::Print() const {
        Text::Print("{}", year);
    }

    bool YearField::Read(BytesMut data) {
        char yearStr[4] = {};
        if (!TagReader::ReadNumeric4Char(data, yearStr)) return false;

        year = (yearStr[0] * 1000 + yearStr[1] * 100 + yearStr[2] * 10 + yearStr[3]) - '0' * 1111;

        return true;
    }

    void DateField::Print() const {
        Text::Print("{}/{}", month, day);
    }

    bool DateField::Read(BytesMut data) {
        char ddmmStr[4] = {};
        if (!TagReader::ReadNumeric4Char(data, ddmmStr)) return false;

        day   = ddmmStr[0] * 10 + ddmmStr[1] - '0' * 11;
        month = ddmmStr[2] * 10 + ddmmStr[3] - '0' * 11;

        return true;
    }

    void TimeField::Print() const {
        Text::Print("{}:{}", hour, min);
    }

    bool TimeField::Read(BytesMut data) {
        char hhmmStr[4] = {};
        if (!TagReader::ReadNumeric4Char(data, hhmmStr)) return false;

        hour = hhmmStr[0] * 10 + hhmmStr[1] - '0' * 11;
        min  = hhmmStr[2] * 10 + hhmmStr[3] - '0' * 11;

        return true;
    }

    void NumberField::Print() const {
        Text::Print("{}", value);
    }

    bool NumberField::Read(BytesMut data) {
        return TagReader::ReadNumericString(data, value);
    }

    void PrintPayload(const TagPayload& payload) {
        payload.Visit([] (const auto& x) { x.Print(); });
    }
#pragma endregion
}

TagReader::TagReader(const char* filename)
    : file(filename, std::ios::in | std::ios::binary) {}

Option<ID3v1> TagReader::ReadV1() {
    file.seekg(-128, std::ios_base::end);
    ID3v1 tag = {};
    file.read((char*)&tag, sizeof(ID3v1));

    // header MUST BE "TAG"
    if ((Memory::ReadU32Big(tag._header) >> 8) != "TAG"_u32) {
        return nullptr;
    } else return tag;
}

bool TagReader::ReadV2Header(ID3v2::Metadata& meta) {
    using namespace ID3v2;
    Header header = {};
    file.read((char*)&header, sizeof(Header));

    // header.fileIdentifier MUST BE "ID3"
    if ((Memory::ReadU32Big(header._fileIdentifier) >> 8) != "ID3"_u32)
        return false;

    // version check
    meta.version = header.versionMajor << 8 | header.versionMinor;
    if (meta.version > 0x03'00) {
        Debug::QError$("encountered ID3 tag version v2.{}.{}, which is higher than supported v2.3.0, aborting", (int)header.versionMajor, (int)header.versionMinor);
        return false;
    }

    // flags check
    if (header.flags & Header::SHOULD_BE_ZEROS) {
        Debug::QError$("bad flag bits. the bottom 5 bits should'nt be set!");
        return false;
    } else if (header.flags & ~Header::SHOULD_BE_ZEROS) {
       Debug::QWarn$("additional flags are not supported.");
    }
    // meta.isUnsynced     = header.flags & Header::UNSYNC;
    // meta.isExtended     = header.flags & Header::EXTENDED_HEADER;
    // meta.isExperimental = header.flags & Header::EXPERIMENTAL;

    if (header.size[0] & 0x80 ||
        header.size[1] & 0x80 ||
        header.size[2] & 0x80 ||
        header.size[3] & 0x80) {
        Debug::QError$("bad header. 8th bit should not be set on size field!");
        return false;
    }

    static constexpr u8 L7 = 0x7F; // last 7 bits
    // base 128, ignore 8th bit
    meta.size = (header.size[0] & L7) << 21 |
                (header.size[1] & L7) << 14 |
                (header.size[2] & L7) << 7 |
                (header.size[3] & L7);
    return true;
}

bool TagReader::ReadV2TagHeader(ID3v2::Tag& tag) {
    using namespace ID3v2;
    // Frame ID       $xx xx xx xx (four characters)
    // Size           $xx xx xx xx
    // Flags          $xx xx
    FrameHeader fHeader = {};
    file.read((char*)&fHeader, sizeof(FrameHeader));

    // id check
    const u32 idBytes = Memory::ReadU32Big(fHeader.id);
    if (('X' << 24) <= idBytes && idBytes <= ('Z' << 24)) {
        const u32 top = (idBytes & 0xFF'00'00'00) - ('X' << 24),
                  btm = (idBytes & 0x00'FF'FF'FF);
        tag.id = (TagID)(top | btm);
    } else if (const auto [found, idx] = Span(TAG_ID_NAMES_INDEX).BinarySearch(idBytes); found) {
        tag.id = (TagID)(idx + 1);
    } else if (idBytes == 0) { // within padding
        tag.id = TagID::NONE;
        return true; // no need to raise an error
    } else {
        tag.id = TagID::NONE;
        Debug::QError$("found unidentifiable tag '{}'", Str::Slice(fHeader.id, 4));
        return false;
    }
    tag.size = Memory::ReadU32Big(fHeader.size);

    // flags check
    if (fHeader.wFlags & FrameHeader::SHOULD_BE_ZEROS || fHeader.rFlags & FrameHeader::SHOULD_BE_ZEROS) {
        Debug::QError$("found bad flags in tag '{}'", Str::Slice(fHeader.id, 4));
        return false;
    }
    const u32 flags = (fHeader.wFlags >> 5) | (fHeader.rFlags >> 2);

    // flag management
    u32 skip = 0;
    if (flags & (1 << 2)) {
        Debug::QWarn$("compression is not yet supported.");
        skip += 4;
    }
    if (flags & (1 << 1)) {
        Debug::QWarn$("encryption is not yet supported.");
        skip += 1;
    }
    if (flags & (1 << 0)) {
        Debug::QWarn$("groups are not yet supported.");
        skip += 1;
    }
    file.seekg(skip, std::ios_base::cur);

    return true;
}

bool TagReader::ReadV2TagData(const ID3v2::Tag& t, Out<ID3v2::TagPayload&> payload) {
    using namespace ID3v2;
    const TagRules& rules = GetTagRules(t.id);
    if (!rules.supported) {
        Debug::QWarn$("tag '{}' ({}) is currently not supported", (const char*)TAG_ID_NAME_LOOKUP[(u32)t.id - 1], rules.properName);
        file.seekg(t.size, std::ios_base::cur);
        return true;
    }
    if (rules.deprecated) {
        Debug::QWarn$("tag '{}' ({}) is deprecated in version 4.0", (const char*)TAG_ID_NAME_LOOKUP[(u32)t.id - 1], rules.properName);
    }

    ArrayBox<byte> payloadBinary = ReadTagPayload(t.size);

    switch (rules.format) {
        case TagFormat::TEXT: {
            payload.Set(TextField());
            return payload.As<TextField>()->Read(payloadBinary);
        }
        case TagFormat::YEAR: {
            payload.Set(YearField());
            return payload.As<YearField>()->Read(payloadBinary);
        }
        case TagFormat::NCHAR: {
            payload.Set(NumberField());
            return payload.As<NumberField>()->Read(payloadBinary);
        }
        case TagFormat::OTHER: switch (t.id) {
            case TagID::UFID: {
                payload.Set(UFID());
                return payload.As<UFID>()->Read(payloadBinary);
            }
            case TagID::TXXX: { // custom text field
                payload.Set(CustomTextField());
                return payload.As<CustomTextField>()->Read(payloadBinary);
            }
            case TagID::TDAT: { // date
                payload.Set(DateField());
                return payload.As<DateField>()->Read(payloadBinary);
            }
            case TagID::TIME: { // time
                payload.Set(TimeField());
                return payload.As<TimeField>()->Read(payloadBinary);
            }
            default: return true;
        }
        default: return false;
    }
}

ArrayBox<byte> TagReader::ReadTagPayload(u32 size) {
    ArrayBox<byte> payload = ArrayBox<byte>::AllocateUninit(size);
    file.read((char*)payload.Data(), size);
    return payload;
}

bool TagReader::ReadTextWithEncoding(bool isUtf16, BytesMut string, String& result, u32& bytesRead) {
    if (isUtf16) {
        Span<u16> words = string.Transmute<Text::Utf16>().SplitOnce(0)[1_st];
        bytesRead = (words.Length() + 1) * 2;
        // detect bom
        const u16 bom = words[0];
        if (bom == 0xFFFE) { // incorrect endianness; need to byteswap
            for (u32 i = 1; i < words.Length(); ++i) {
                Memory::WriteU16(words[i], &words[i]);
            }
        }
        auto txt = Text::Utf16To8(words.Skip(1));
        if (!txt) {
            Debug::QError$("ill-formed UTF-16 string!");
            return false;
        }
        result = std::move(*txt);
    } else {
        const Bytes nullTerminated = string.SplitOnce(0)[1_st];
        bytesRead = nullTerminated.Length() + 1;
        result = Text::Latin1ToUtf8(nullTerminated.Transmute<Text::Latin1>());
    }
    return true;
}

bool TagReader::ReadNumeric4Char(BytesMut string, char(& result)[4]) {
    const bool isUTF16 = string[0];
    if ((isUTF16 && string.Length() != 11) || (!isUTF16 && string.Length() != 6)) {
        Debug::QWarn$("field should be 4 characters long! found '{}'", string.AsStr().Subspan(1, isUTF16 ? 8 : 4));
        return false;
    }

    if (isUTF16) {
        result[0] = string[4]  | string[5];
        result[1] = string[6]  | string[7];
        result[2] = string[8]  | string[9];
        result[3] = string[10] | string[11];
    } else {
        Memory::MemCopy(result, string.Data() + 1, 4);
    }

    if (!Chr::IsDigit(result[0]) || !Chr::IsDigit(result[1])
     || !Chr::IsDigit(result[2]) || !Chr::IsDigit(result[3])) {
        Debug::QWarn$("bad year/date/time field! found '{}'", Str::Slice(result, 4));
        return false;
    }

    return true;
}

bool TagReader::ReadNumericString(BytesMut string, u64& result) {
    const bool isUTF16 = string[0];
    string.Advance(1);
    if (isUTF16) {
        // bom is either: FEFF or FFFE
        // if bom is FEFF then just read second digit.
        const u32 b = string[1] == 0xFE;
        result = 0;
        for (u32 i = 2 + b; i < string.Length() - 2; i += 2) {
            if (string[i] == 0) break;
            if (!Chr::IsDigit(string[i])) return false;
            result *= 10;
            result += string[i] - '0';
        }
    } else {
        // remove null term
        if (string.Last() != 0) {
            Debug::QError$("no null terminator in string!");
            return false;
        }
        if (const auto res = Text::Parse<u64>(string.First(string.Length() - 1).AsStr())) {
            result = *res;
        } else return false;
    }
    return true;
}

// bool TagReader::ReadV2URLField(BytesMut data, String& urlOut) const {
//     // <Header for 'URL link frame', ID: "W000" - "WZZZ", excluding "WXXX" described in 4.3.2.>
//     // URL <text string>
//     urlOut = String((const char*)data.Data());
//     return true;
// }
//
// bool TagReader::ReadV2UseURLField(BytesMut data, String& descOut, String& urlOut) const {
//     // <Header for 'User defined URL link frame', ID: "WXXX">
//     // Text encoding    $xx
//     // Description    <text string according to encoding> $00 (00)
//     // URL    <text string>
//     const bool isUTF16 = data[0];
//
//     const auto nullTerm = FindNullTerminator(isUTF16, data.Skip(1));
//     if (!nullTerm) {
//         Debug::QError$("no null terminator in user defined URL field!");
//         return false;
//     }
//
//     return ReadTextWithEncoding(isUTF16, data.Subspan(1, *nullTerm), descOut)
//         && ReadTextWithEncoding(false, data.Subspan(*nullTerm + 2), urlOut);
// }

Option<ID3v2::Metadata> TagReader::ReadV2() {
    using namespace ID3v2;
    file.seekg(0, std::ios_base::beg);

    Metadata meta = {};
    if (!ReadV2Header(meta)) return nullptr;

    for (usize readSize = 0; readSize < meta.size;) {
        Tag tag = {};
        if (!ReadV2TagHeader(tag)) return nullptr;
        if (tag.id == TagID::NONE) break; // just skip everything; we've hit padding

        meta.tags.Push(tag);
        if (!ReadV2TagData(tag, meta.tagData.Push(None {}))) return nullptr;

        readSize += sizeof(FrameHeader) + tag.size;
    }
    return meta;
}
