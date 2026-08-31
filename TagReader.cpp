#include "TagReader.h"

#include "Utils/Debug/Logger.h"
#include "Utils/Algorithm.h"
#include "Utils/Array.h"
#include "Utils/Bitwise.h"
#include "Utils/Text/UTF.h"

namespace ID3v2 {
#pragma region Tag Rules
    constexpr bool YES = true, NO = false;
    constexpr TagRules TAG_RULES[] = {
        // X = will not add, U = possibly
        // [allowsMultiple] [supported] [deprecated] [supportedMajorVersion]
        { YES, NO,  NO,  3, "audioEncryption",  "Audio Encryption",  TagFormat::OTHER }, // X AENC: the encryption method of the audio ???
        { YES, YES, NO,  3, "cover",            "Cover",             TagFormat::OTHER }, //   APIC?: cover pictures, can also include other pictures. see section 4.15
        { YES, NO,  NO,  3, "comment",          "Comment",           TagFormat::OTHER }, //   COMM: comments, format: [ENC (1b)] [LANG (3b)] [CONTENT DESCRIPTOR (?b)] \0 [TEXT (?b) W/ NEWLINES]
        { NO,  NO,  NO,  3, "purchaseInfo",     "Purchase Info",     TagFormat::OTHER }, // X COMR: about the purchase
        { YES, NO,  NO,  3, "__encryption",     "_Encryption",       TagFormat::OTHER }, // X ENCR: about the encryption info of other fields
        { NO,  NO,  YES, 3, "equalization",     "Equalization",      TagFormat::OTHER }, // X EQUA: equalize each frequency. complicated, see section 4.13
        { NO,  NO,  NO,  3, "events",           "Events",            TagFormat::OTHER }, // U ETCO: list of events like opening, outro... see section 4.6
        { YES, NO,  NO,  3, "extraData",        "Extra Data",        TagFormat::OTHER }, // X GEOB: arbitrary extra files attached to the mp3
        { YES, NO,  NO,  3, "__grouping",       "_Grouping",         TagFormat::OTHER }, // X GRID: grouping info
        { NO,  NO,  YES, 3, "involvedPeople",   "Involved People",   TagFormat::OTHER }, //   IPLS: everybody involved, format: person - involvment\0person2 - involvement\0...
        { YES, NO,  NO,  3, "__link",           "_Link",             TagFormat::OTHER }, // X LINK: 'if you see this, grab a copy of a specific field from a different file and paste it here'
        { NO,  NO,  NO,  3, "cd",               "CD",                TagFormat::OTHER }, // X MCDI: cd, so it can be identified in a database like CDDB. binary dump, see section 4.5
        { NO,  NO,  NO,  3, "_mpgLocationLT",   "_MLLT",             TagFormat::OTHER }, // X MLLT: ??? section 4.7
        { NO,  NO,  NO,  3, "ownershipProof",   "Ownership Proof",   TagFormat::OTHER }, // X OWNE: proof of ownership. may include transaction info, payment amount...
        { NO,  NO,  NO,  3, "playCount",        "Play Count",        TagFormat::OTHER }, //   PCNT: play count, >=5 bytes
        { NO,  NO,  NO,  3, "rating",           "Rating",            TagFormat::OTHER }, // U POPM: 'popularity' for a song. see section 4.18
        { NO,  NO,  NO,  3, "playPosition",     "Play Position",     TagFormat::OTHER }, // X POSS: what offset the song should be played. intended for unfinished listens and may want to listen again
        { YES, NO,  NO,  3, "privateData",      "Private Data",      TagFormat::OTHER }, // X PRIV: misc. info...
        { NO,  NO,  NO,  3, "preferredBufSize", "Pref. Buffer Size", TagFormat::OTHER }, // X RBUF: recommended buffer size when streaming
        { NO,  NO,  YES, 3, "volumeAdjustment", "Volume Adjustment", TagFormat::OTHER }, // X RVAD: volume adjustment. complicated, see section 4.12
        { NO,  NO,  NO,  3, "reverb",           "Reverb",            TagFormat::OTHER }, // X RVRB: reverb amount. complicated, see section 4.14
        { YES, YES, NO,  3, "syncedLyrics",     "Synced Lyrics",     TagFormat::OTHER }, //   SYLT: synced lyrics
        { NO,  NO,  NO,  3, "tempos",           "Tempos",            TagFormat::OTHER }, // U SYTC: list of (bpm, timestamp) pairs, section 4.8
        { NO,  YES, NO,  3, "album",            "Album",             TagFormat::TEXT  }, //   TALB: (song source, could be album/movie/show),
        { NO,  YES, NO,  3, "bpm",              "BPM",               TagFormat::NCHAR }, //   TBPM: (bpm, number format)
        { NO,  YES, NO,  3, "composer",         "Composer",          TagFormat::TEXT  }, //   TCOM: (composers, list format, separated by a '/')
        { NO,  YES, NO,  3, "genre",            "Genre",             TagFormat::TEXT  }, // U TCON: (see section 4.2.1),
        { NO,  YES, NO,  3, "copyrightYear",    "Copyright Year",    TagFormat::TEXT  }, //   TCOP: (should be a year + a space + ... (>= 5 chars). when displayed, should be "Copyright © [this value]")
        { NO,  YES, YES, 3, "date",             "Date",              TagFormat::OTHER }, //   TDAT: (format: DDMM, always 4 chars)
        { NO,  YES, NO,  3, "playlistDelay",    "Playlist Delay",    TagFormat::NCHAR }, // X TDLY: (amount of delay (milliseconds) when playing between songs, number format)
        { NO,  YES, NO,  3, "encodedBy",        "Encoded By",        TagFormat::TEXT  }, // X TENC: (encoding origin, may be person/org. may contain copyright)
        { NO,  YES, NO,  3, "lyricist",         "Lyricist",          TagFormat::TEXT  }, //   TEXT: (the lyricists of the song; list format)
        { NO,  YES, NO,  3, "fileType",         "File Type",         TagFormat::TEXT  }, // U TFLT: (can be: MPG (default)|/1|/2|/3|/2.4|/AAC|VQF|PCM)
        { NO,  YES, YES, 3, "time",             "Time",              TagFormat::OTHER }, //   TIME: (foramt: HHMM, always 4 chars)
        { NO,  YES, NO,  3, "category",         "Category",          TagFormat::TEXT  }, // U TIT1: (song category, similar to genre but more broad)
        { NO,  YES, NO,  3, "name",             "Name",              TagFormat::TEXT  }, //   TIT2: (name of the song)
        { NO,  YES, NO,  3, "description",      "Description",       TagFormat::TEXT  }, //   TIT3: (description)
        { NO,  YES, NO,  3, "firstKey",         "First Key",         TagFormat::TEXT  }, // X TKEY: (first key in the music, can be ([A-G]?[b#]?[m])|o)
        { NO,  YES, NO,  3, "language",         "Language",          TagFormat::TEXT  }, //   TLAN: (language code, 3 chars ISO-639-2)
        { NO,  YES, NO,  3, "duration",         "Duration",          TagFormat::NCHAR }, //   TLEN: (duration of song (milliseconds), number format)
        { NO,  YES, NO,  3, "mediaType",        "Media Type",        TagFormat::TEXT  }, // U TMED: (song origin ex: (DIG) for digital. see section 4.2.1)
        { NO,  YES, NO,  3, "originalAlbum",    "Original Album",    TagFormat::TEXT  }, // X TOAL: (original song source, if it was readapted/remix/cover of a different album)
        { NO,  YES, NO,  3, "originalName",     "Original Name",     TagFormat::TEXT  }, // X TOFN: (original filename, if the actual filename had to be changed to abide by restrictions)
        { NO,  YES, NO,  3, "originalLyricist", "Original Lyricist", TagFormat::TEXT  }, // X TOLY: (original lyricist, for adapted songs, list format)
        { NO,  YES, NO,  3, "originalArtists",  "Original Artists",  TagFormat::TEXT  }, // X TOPE: (original artists/performers, list format)
        { NO,  YES, YES, 3, "originalYear",     "Original Year",     TagFormat::TEXT  }, // X TORY: (original release year, 4 chars)
        { NO,  YES, NO,  3, "owner",            "Owner",             TagFormat::TEXT  }, // X TOWN: (file owner/liscensee)
        { NO,  YES, NO,  3, "artists",          "Artists",           TagFormat::TEXT  }, //   TPE1: (artists/performers, list format)
        { NO,  YES, NO,  3, "band",             "Band",              TagFormat::TEXT  }, //   TPE2: (additional info for artists)
        { NO,  YES, NO,  3, "conductor",        "Conductor",         TagFormat::TEXT  }, // U TPE3: (conductor)
        { NO,  YES, NO,  3, "remixedBy",        "Remixed By",        TagFormat::TEXT  }, // X TPE4: (info about remix creator)
        { NO,  YES, NO,  3, "part",             "Part",              TagFormat::NCHAR }, // U TPOS: (which part the audio came from, if the album contains many mediums
        { NO,  YES, NO,  3, "publisher",        "Publisher",         TagFormat::TEXT  }, // X TPUB: (publisher)
        { NO,  YES, NO,  3, "trackNumber",      "Track Number",      TagFormat::NCHAR }, // U TRCK: (#, same format as TPOS)
        { NO,  YES, NO,  3, "recordedDate",     "Recorded Date",     TagFormat::TEXT  }, // X TRDA: (complement to other date info, any text no specific format, ex: 4th-7th June)
        { NO,  YES, NO,  3, "radioName",        "Radio Name",        TagFormat::TEXT  }, // X TRSN: (name of radio station which was streamed from)
        { NO,  YES, NO,  3, "radioOwner",       "Radio Owner",       TagFormat::TEXT  }, // X TRSO: (radio station owner)
        { NO,  YES, YES, 3, "filesize",         "Filesize",          TagFormat::NCHAR }, // X TSIZ: (file size excluding ID3v2, in bytes)
        { NO,  YES, NO,  3, "irscCode",         "ISRC Code",         TagFormat::TEXT  }, // X TSRC: (international standard recording code, 12 chars)
        { NO,  YES, NO,  3, "encoderSettings",  "Encoder Settings",  TagFormat::TEXT  }, // X TSSE: (settings for audio encoder used)
        { NO,  YES, NO,  3, "extraData",        "Extra Data",        TagFormat::OTHER }, //   TXXX: (user defined)
        { NO,  YES, YES, 3, "year",             "Year",              TagFormat::YEAR  }, //   TYER: (year, 4 chars)
        { YES, YES, NO,  3, "ufid",             "UFID",              TagFormat::OTHER }, //   UFID: unique file identifier, binary
        { NO,  NO,  NO,  3, "termsOfUse",       "Terms Of Use",      TagFormat::OTHER }, // X USER: terms of use
        { YES, YES, NO,  3, "unsyncedLyrics",   "Unsynced Lyrics",   TagFormat::OTHER }, //   USLT: unsynced lyrics
        { YES, NO,  NO,  3, "comercialInfo",    "Comercial Info",    TagFormat::OTHER }, // X WCOM: (where you can buy the album, may be multiple)
        { NO,  NO,  NO,  3, "copyright",        "Copyright",         TagFormat::OTHER }, // X WCOP: (terms of use & ownership)
        { NO,  NO,  NO,  3, "webpage",          "Webpage",           TagFormat::OTHER }, // X WOAF: (official audio webpage)
        { YES, NO,  NO,  3, "artistWebpage",    "Artist Webpage",    TagFormat::OTHER }, // X WOAR: (official artist webpage)
        { NO,  NO,  NO,  3, "sourceWebpage",    "Source Webpage",    TagFormat::OTHER }, // X WOAS: (official source/album/movie/show webpage)
        { NO,  NO,  NO,  3, "radioWebpage",     "Radio Webpage",     TagFormat::OTHER }, // X WORS: (official radio station webpage)
        { NO,  NO,  NO,  3, "payment",          "Payment",           TagFormat::OTHER }, // X WPAY: (payment handling webpage)
        { NO,  NO,  NO,  3, "publisherWebpage", "Publisher Webpage", TagFormat::OTHER }, // X WPUB: (publisher webpage)
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
    // APIC*?: cover pictures, can also include other pictures. see section 4.15
    // USLT*:  unsynced lyrics, format: [ENCODING (1b)] [LANGUAGE (3b)] [CONTENT DESCRIPTOR (?b)] \0 [LYCRIS W/ NEWLINES]
    // SYLT*:  synced lyrics, format: [ENC (1b)] [LANG (3b)] [TIMEFMT (1b)] [CONTENTTYPE (1b)] [DESC (?b)] \0 [TEXT_CHUNK\0TIME...]
    // TODO TO ADD:
    //     EASY:
    //         PCNT : play count, >=4 bytes
    //     TRICKY:
    //         COMM*: comments, format: [ENC (1b)] [LANG (3b)] [CONTENT DESCRIPTOR (?b)] \0 [TEXT (?b) W/ NEWLINES]
    //         IPLS : everybody involved, format: person - involvment\0person2 - involvement\0...
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

    void PictureField::Print() const {
        Text::Print("[type = {}, format = {}, desc = {}, ...]",
            (u32)pictureType, fileType == 0 ? "'.png'" : "'.jpeg'", desc);
    }

    bool PictureField::Read(BytesMut data) {
        const bool isUTF16 = data[0];
        data.Advance(1);

        String mimeType;
        u32 bytesRead = 0;
        TagReader::ReadTextWithEncoding(false, data, mimeType, bytesRead);
        data.Advance(bytesRead);

        const Str mime = mimeType.RemovePrefix("image/");
        if (mime == "png") {
            fileType = 0;
        } else if (mime == "jpeg") {
            fileType = 1;
        } else {
            Debug::QError$("found unsupported image file type '{}'", mimeType);
            return false;
        }

        pictureType = data[0];
        data.Advance(1);

        if (!TagReader::ReadTextWithEncoding(isUTF16, data, desc, bytesRead)) return false;
        data.Advance(bytesRead);

        pictureData = ArrayBox<byte>::New(data);
        return true;
    }

    Array<char, 3> GetLangCode(Language lang) {
        return { (char)(lang >> 16), (char)(lang >> 8), (char)(lang) };
    }

    void UnsyncedLyricsField::Print() const {
        const auto langCode = GetLangCode(lang);
        const Str langName = Str::Slice(langCode.Data(), 3);
        if (desc)
            Text::Print("(lang: {}; {:?}) {}", langName, desc, lyrics);
        else
            Text::PrintLn("(lang: {}) {}", langName, lyrics);
    }

    bool UnsyncedLyricsField::Read(BytesMut data) {
        // section 4.9
        // Text encoding       $xx
        // Language            $xx xx xx
        // Content descriptor  <text string according to encoding> $00 (00)
        // Lyrics/text         <full text string according to encoding>
        const bool isUTF16 = data[0];
        lang = (Language)(Memory::ReadU32Big(data.Data()) & 0xFFFFFF);
        data.Advance(4);
        u32 bytesRead;
        if (!TagReader::ReadTextWithEncoding(isUTF16, data, desc, bytesRead)) return false;
        data.Advance(bytesRead);
        return TagReader::ReadTextWithEncoding(isUTF16, data, lyrics, bytesRead);
    }

    void SyncedLyrics::Print() const {
        Bytes rawBytes = raw;
        while (rawBytes) {
            u32 miliseconds = Memory::ReadU32Native(rawBytes.Data()),
                wordLength = Memory::ReadU32Native(rawBytes.Data() + sizeof(u32));
            rawBytes.Advance(sizeof(u32) * 2);
            Str word = rawBytes.First(wordLength).AsStr();
            rawBytes.Advance(wordLength);

            u32 seconds = miliseconds / 1000, minutes = seconds / 60, hours = minutes / 60;
            miliseconds -= seconds * 1000;
            seconds -= minutes * 60;
            minutes -= hours * 60;

            Text::Print("\n[{:02}:{:02}:{:02}.{:03}] {}", hours, minutes, seconds, miliseconds, word);
        }
    }

    bool SyncedLyrics::Read(bool isUtf16, BytesMut data) {
        u32 i = 0;
        String word;

        while (data) {
            if (!TagReader::ReadTextWithEncoding(isUtf16, data, word, i)) return false;
            data.Advance(i);

            // first 4 for timestamp; last 4 for word length
            byte timestampAndLen[8];
            Memory::WriteU32Native(Memory::ReadU32Big(data.Data()), timestampAndLen);
            Memory::WriteU32Native(word.Length(),                   timestampAndLen + 4);

            raw.Extend(timestampAndLen);
            raw.Extend(word.AsBytes());

            data.Advance(sizeof(u32));
        }
        return true;
    }

    void SyncedLyricsField::Print() const {
        const auto langCode = GetLangCode(lang);
        const Str langName = Str::Slice(langCode.Data(), 3);
        if (desc)
            Text::Print("(lang: {}; content: #{}; {:?})", langName, (int)type, desc);
        else
            Text::PrintLn("(lang: {}; content: #{})", langName, (int)type);
        lyrics.Print();
    }

    bool SyncedLyricsField::Read(BytesMut data) {
        // Text encoding       $xx
        // Language            $xx xx xx
        // Time stamp format   $xx
        // Content type        $xx
        // Content descriptor  <text string according to encoding> $00 (00)
        const bool isUTF16 = data[0];
        lang = (Language)(Memory::ReadU32Big(data.Data()) & 0xFFFFFF);

        switch (data[4]) {
            case 1: timeUnit = TimeStampUnit::FRAMES; break;
            case 2: timeUnit = TimeStampUnit::MILLISECONDS; break;
            default: Debug::QError$("bad time unit!"); break;
        }
        type = (SyncedContentType)data[5];

        data.Advance(6);

        u32 bytesRead;
        if (!TagReader::ReadTextWithEncoding(isUTF16, data, desc, bytesRead)) return false;
        data.Advance(bytesRead);

        return lyrics.Read(isUTF16, data);
    }

    void PrintPayload(const TagPayload& payload) {
        payload.Visit([] (const auto& x) { x.Print(); });
    }
#pragma endregion
}

TagReader::TagReader(const char* filename)
    : file(filename, std::ios::in | std::ios::binary) {
    if (file.fail()) {
        Debug::QError$("Failed to open file {}!", filename);
    }
}

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
    meta.tagVersion = header.versionMajor << 8 | header.versionMinor;
    if (meta.tagVersion > 0x03'00) {
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
    if (rules.deprecatedInV4) {
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
            case TagID::APIC: { // picture
                payload.Set(PictureField());
                return payload.As<PictureField>()->Read(payloadBinary);
            }
            case TagID::USLT: {
                payload.Set(UnsyncedLyricsField());
                return payload.As<UnsyncedLyricsField>()->Read(payloadBinary);
            }
            case TagID::SYLT: {
                payload.Set(SyncedLyricsField());
                return payload.As<SyncedLyricsField>()->Read(payloadBinary);
            }
            default: return true;
        }
        default: return false;
    }
}

bool TagReader::ReadMpegData(ID3v2::Metadata& meta) {
    static constexpr usize READ_BATCH_SIZE = 1024;
    byte buffer[READ_BATCH_SIZE + 3];

    // http://www.mp3-tech.org/programmer/frame_header.html
    // find 0xFFE or 0xFFF (beginning of header)
    int i = 0;
    while (true) {
        file.read((char*)buffer, READ_BATCH_SIZE + 3);

        for (i = 0; i < READ_BATCH_SIZE; i++) {
            if (buffer[i] == 0xFF && (buffer[i + 1] & 0xE0) == 0xE0) {
                goto foundHeader;
            }
        }

        file.seekg(READ_BATCH_SIZE, std::ios_base::cur);
        if (file.fail()) {
            return false;
        }
    }
    foundHeader:

    // AAAAAAAA AAABBCCD EEEEFFGH IIJJKLMM
    constexpr u32 VERSION_MASK  = 0x00'18'00'00, // B
                  LAYER_MASK    = 0x00'06'00'00, // C
                  BITRATE_MASK  = 0x00'00'F0'00, // E
                  SAMPLING_MASK = 0x00'00'0C'00, // F
                  CHANNELS_MASK = 0x00'00'00'C0; // I

    // 00; 01; 10; 11
    static constexpr ID3v2::MpegVersion RESERVED_VERSION = (ID3v2::MpegVersion)~0;
    static constexpr ID3v2::MpegVersion VERSION_TABLE[4] = { ID3v2::V2_5, RESERVED_VERSION, ID3v2::V2, ID3v2::V1 };
    static constexpr u8 LAYER_TABLE[4] = { 0, 3, 2, 1 };

    const u32 header = Memory::ReadU32Big(buffer + i);
    meta.mpgVersion   = VERSION_TABLE[(header & VERSION_MASK) >> u32s::CountRightZeros(VERSION_MASK)];
    meta.layerVersion = LAYER_TABLE[(header & LAYER_MASK) >> u32s::CountRightZeros(LAYER_MASK)];
    if (meta.mpgVersion == RESERVED_VERSION || meta.layerVersion == 0) {
        Debug::QError$("bad frame header mpeg version or layer version!");
        return false;
    }

    // bitrate
    static constexpr u16 FREE = 0, BAD = u16s::MAX;
    static constexpr u16 BITRATE_TABLE[16 * 5] = {
             /* V1,L1 	V1,L2 	V1,L3 	V2,L1 	V2, L2 & L3 */
    /* 0000 */ 	FREE, 	FREE, 	FREE, 	FREE, 	FREE,
    /* 0001 */ 	32,     32, 	32, 	32, 	8,
    /* 0010 */ 	64,     48, 	40, 	48, 	16,
    /* 0011 */ 	96,     56, 	48, 	56, 	24,
    /* 0100 */ 	128, 	64, 	56, 	64, 	32,
    /* 0101 */ 	160, 	80, 	64, 	80, 	40,
    /* 0110 */ 	192, 	96, 	80, 	96, 	48,
    /* 0111 */ 	224, 	112, 	96, 	112, 	56,
    /* 1000 */ 	256, 	128, 	112, 	128, 	64,
    /* 1001 */ 	288, 	160, 	128, 	144, 	80,
    /* 1010 */ 	320, 	192, 	160, 	160, 	96,
    /* 1011 */ 	352, 	224, 	192, 	176, 	112,
    /* 1100 */ 	384, 	256, 	224, 	192, 	128,
    /* 1101 */ 	416, 	320, 	256, 	224, 	144,
    /* 1110 */ 	448, 	384, 	320, 	256, 	160,
    /* 1111 */ 	BAD, 	BAD, 	BAD, 	BAD, 	BAD,
    };
    // bitrate
    const bool isV2Plus = meta.mpgVersion >= ID3v2::V2;
    const u32 bitrateID = (header & BITRATE_MASK) >> u32s::CountRightZeros(BITRATE_MASK),
              columnID  = isV2Plus ? (meta.layerVersion == 1 ? 3 : 4) : (meta.layerVersion - 1);
    meta.bitrateKbps = BITRATE_TABLE[bitrateID * 5 + columnID];
    if (meta.bitrateKbps == BAD) {
        Debug::QError$("bad bitrate!");
        return false;
    }

    // sampling freq.
    static constexpr u16 RESERV = ~0; // reserv.
    static constexpr u16 SAMPLING_RATE_TABLE[4 * 3] = {
        /* bits     MPEG1    MPEG2   MPEG2.5 */
        /* 00   */ 	44100, 	 22050,  11025,
        /* 01   */ 	48000, 	 24000,  12000,
        /* 10   */ 	32000, 	 16000,  8000,
        /* 11   */ 	RESERV,  RESERV, RESERV,
    };

    const u32 samplingID = (header & SAMPLING_MASK) >> u32s::CountRightZeros(SAMPLING_MASK);
    meta.samplingRate = SAMPLING_RATE_TABLE[samplingID * 3 + (meta.mpgVersion - 1)];
    if (meta.samplingRate == RESERV) {
        Debug::QError$("bad sampling rate!");
        return false;
    }

    meta.channels = (header & CHANNELS_MASK) == CHANNELS_MASK ? 1 : 2;

    return true;
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

    ReadMpegData(meta);

    return meta;
}
