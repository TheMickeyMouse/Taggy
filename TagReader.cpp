#include "TagReader.h"

#include "Utils/Debug/Logger.h"
#include "Utils/Algorithm.h"
#include "Utils/Text/UTF.h"

namespace ID3v2 {
    constexpr const char* TAG_NAMES_JSON[] = {
        // X = will not add, U = possibly
        "audioEncryption",  // X AENC*: the encryption method of the audio ???
        "cover",            //   APIC*?: cover pictures, can also include other pictures. see section 4.15
        "comment",          //   COMM*: comments, format: [ENC (1b)] [LANG (3b)] [CONTENT DESCRIPTOR (?b)] \0 [TEXT (?b) W/ NEWLINES]
        "purchaseInfo",     // X COMR : about the purchase
        "__encryption",     // X ENCR*: about the encryption info of other fields
        "equalization",     // X EQUA : equalize each frequency. complicated, see section 4.13
        "events",           // U ETCO : list of events like opening, outro... see section 4.6
        "extraData",        // X GEOB*: arbitrary extra files attached to the mp3
        "__grouping",       // X GRID*: grouping info
        "involvedPeople",   //   IPLS : everybody involved, format: person - involvment\0person2 - involvement\0...
        "__link",           // X LINK*: 'if you see this, grab a copy of a specific field from a different file and paste it here'
        "cd",               // X MCDI : cd, so it can be identified in a database like CDDB. binary dump, see section 4.5
        "mpegLocationLT",   // X MLLT : ??? section 4.7
        "ownershipProof",   // X OWNE : proof of ownership. may include transaction info, payment amount...
        "playCount",        //   PCNT : play count, >=4 bytes
        "rating",           // U POPM : 'popularity' for a song. see section 4.18
        "playPosition",     // X POSS : what offset the song should be played. intended for unfinished listens and may want to listen again
        "privateData",      // X PRIV*: misc. info...
        "preferredBufSize", // X RBUF : recommended buffer size when streaming
        "volumeAdjustment", // X RVAD : volume adjustment. complicated, see section 4.12
        "reverb",           // X RVRB : reverb amount. complicated, see section 4.14
        "syncedLyrics",     //   SYLT*: synced lyrics, format: [ENC (1b)] [LANG (3b)] [TIMEFMT (1b)] [CONTENTTYPE (1b)] [DESC (?b)] \0 [TEXT_CHUNK\0TIME...]
        "tempos",           // U SYTC : list of (bpm, timestamp) pairs, section 4.8
        "album",            //   TALB : (song source, could be album/movie/show),
        "bpm",              //   TBPM : (bpm, number format)
        "composer",         //   TCOM : (composers, list format, separated by a '/')
        "genre",            // U TCON : (see section 4.2.1),
        "copyrightYear",    //   TCOP : (should be a year + a space + ... (>= 5 chars). when displayed, should be "Copyright © [this value]")
        "date",             //   TDAT : (format: DDMM, always 4 chars)
        "playlistDelay",    // X TDLY : (amount of delay (milliseconds) when playing between songs, number format)
        "encodedBy",        // X TENC : (encoding origin, may be person/org. may contain copyright)
        "lyricist",         //   TEXT : (the lyricists of the song; list format)
        "fileType",         // U TFLT : (can be: MPG (default)|/1|/2|/3|/2.4|/AAC|VQF|PCM)
        "time",             //   TIME : (foramt: HHMM, always 4 chars)
        "category",         // U TIT1 : (song category, similar to genre but more broad)
        "name",             //   TIT2 : (name of the song)
        "description",      //   TIT3 : (description)
        "firstKey",         // X TKEY : (first key in the music, can be ([A-G]?[b#]?[m])|o)
        "language",         //   TLAN : (language code, 3 chars ISO-639-2)
        "duration",         //   TLEN : (duration of song (milliseconds), number format)
        "mediaType",        // U TMED : (song origin ex: (DIG) for digital. see section 4.2.1)
        "originalAlbum",    // X TOAL : (original song source, if it was readapted/remix/cover of a different album)
        "originalName",     // X TOFN : (original filename, if the actual filename had to be changed to abide by restrictions)
        "originalLyricist", // X TOLY : (original lyricist, for adapted songs, list format)
        "originalArtists",  // X TOPE : (original artists/performers, list format)
        "originalYear",     // X TORY : (original release year, 4 chars)
        "owner",            // X TOWN : (file owner/liscensee)
        "artists",          //   TPE1 : (artists/performers, list format)
        "band",             //   TPE2 : (additional info for artists)
        "conductor",        // U TPE3 : (conductor)
        "remixedBy",        // X TPE4 : (info about remix creator)
        "part",             // U TPOS : (which part the audio came from, if the album contains many mediums. format: num/total (latter and slash is option))
        "publisher",        // X TPUB : (publisher)
        "trackNumber",      // U TRCK : (#, same format as TPOS)
        "recordedDate",     // X TRDA : (complement to other date info, any text no specific format, ex: 4th-7th June)
        "radioName",        // X TRSN : (name of radio station which was streamed from)
        "radioOwner",       // X TRSO : (radio station owner)
        "filesize",         // X TSIZ : (file size excluding ID3v2, in bytes)
        "irscCode",         // X TSRC : (international standard recording code, 12 chars)
        "encoderSettings",  // X TSSE : (settings for audio encoder used)
        "extraData",        //   TXXX : (user defined)
        "year",             //   TYER : (year, 4 chars)
        "ufid",             //   UFID*: unique file identifier, binary
        "termsOfUse",       // X USER : terms of use
        "unsyncedLyrics",   //   USLT*: unsynced lyrics, format: [ENCODING (1b)] [LANGUAGE (3b)] [CONTENT DESCRIPTOR (?b)] \0 [LYCRIS W/ NEWLINES]
        "comercialInfo",    // X WCOM*: (where you can buy the album, may be multiple)
        "copyright",        // X WCOP : (terms of use & ownership)
        "webpage",          // X WOAF : (official audio webpage)
        "artistWebpage",    // X WOAR*: (official artist webpage)
        "sourceWebpage",    // X WOAS : (official source/album/movie/show webpage)
        "radioWebpage",     // X WORS : (official radio station webpage)
        "payment",          // X WPAY : (payment handling webpage)
        "publisherWebpage", // X WPUB : (publisher webpage)
    };

    // ADDED:
    //
    // TODO TO ADD:
    // APIC*?: cover pictures, can also include other pictures. see section 4.15
    // COMM*: comments, format: [ENC (1b)] [LANG (3b)] [CONTENT DESCRIPTOR (?b)] \0 [TEXT (?b) W/ NEWLINES]
    // IPLS : everybody involved, format: person - involvment\0person2 - involvement\0...
    // PCNT : play count, >=4 bytes
    // SYLT*: synced lyrics, format: [ENC (1b)] [LANG (3b)] [TIMEFMT (1b)] [CONTENTTYPE (1b)] [DESC (?b)] \0 [TEXT_CHUNK\0TIME...]
    // TALB : (song source, could be album/movie/show),
    // TBPM : (bpm, number format)
    // TCOM : (composers, list format, separated by a '/')
    // TCOP : (should be a year + a space + ... (>= 5 chars). when displayed, should be "Copyright © [this value]")
    // TDAT : (format: DDMM, always 4 chars)
    // TEXT : (the lyricists of the song; list format)
    // TIME : (foramt: HHMM, always 4 chars)
    // TIT2 : (name of the song)
    // TIT3 : (description)
    // TLAN : (language code, 3 chars ISO-639-2)
    // TLEN : (duration of song (milliseconds), number format)
    // TPE1 : (artists/performers, list format)
    // TPE2 : (additional info for artists)
    // TXXX : (user defined)
    // TYER : (year, 4 chars)
    // UFID*: unique file identifier, binary
    // USLT*: unsynced lyrics, format: [ENCODING (1b)] [LANGUAGE (3b)] [CONTENT DESCRIPTOR (?b)] \0 [LYCRIS W/ NEWLINES]
    // TODO: MIGHT ADD:
    // U ETCO : list of events like opening, outro... see section 4.6
    // U POPM : 'popularity' for a song. see section 4.18
    // U SYTC : list of (bpm, timestamp) pairs, section 4.8
    // U TCON : (see section 4.2.1),
    // U TFLT : (can be: MPG (default)|/1|/2|/3|/2.4|/AAC|VQF|PCM)
    // U TIT1 : (song category, similar to genre but more broad)
    // U TMED : (song origin ex: (DIG) for digital. see section 4.2.1)
    // U TPE3 : (conductor)
    // U TPOS : (which part the audio came from, if the album contains many mediums. format: num/total (latter and slash is option))
    // U TRCK : (#, same format as TPOS)
}


const char* ID3v2::GetTagIDJsonName(TagID id) {
    return TAG_NAMES_JSON[(u32)id - (u32)TagID::AENC];
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

bool TagReader::ReadV2FrameHeader(ID3v2::Tag& tag) {
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

ArrayBox<char> TagReader::ReadTagPayload(u32 size) {
    ArrayBox<char> payload = ArrayBox<char>::AllocateUninit(size);
    file.read(payload.Data(), size);
    return payload;
}

bool TagReader::ReadV2UFID(u32 size, ID3v2::UFID& ufid) {
    const ArrayBox<char> payload = ReadTagPayload(size);
    // UFID:
    // Owner identifier    <text string> 0x00
    // Identifier    <up to 64 bytes binary data>
    ufid.owner = { payload.Data() };
    ufid.identifier = Bytes::Slice((const u8*)ufid.owner.DataEndWithNull(), size - ufid.owner.LengthWithNull());
    return true;
}

bool TagReader::ReadTextWithEncoding(bool isUtf16, BytesMut string, String& result) const {
    if (isUtf16) {
        // detect bom
        const u16 bom1 = Memory::ReadU16Native(&string[0]);
        if (bom1 == 0xFFEE) { // incorrect endianness; need to byteswap
            for (u32 i = 2; i < string.Length(); i += 2) {
                Memory::WriteU16(Memory::ReadU16Big(&string[i]), &string[i]);
            }
        }
        auto txt = Text::Utf16To8(string.Skip(2).Transmute<Text::Utf16>());
        if (!txt) {
            Debug::QError$("ill-formed UTF-16 string!");
            return false;
        }
        result = std::move(*txt);
    } else {
        result = Text::Latin1ToUtf8(string.Transmute<Text::Latin1>());
    }
    return true;
}

OptionUsize TagReader::FindNullTerminator(bool isUtf16, Bytes string) const {
    OptionUsize nullTerm;
    if (isUtf16) {
        for (usize i = 0; i < string.Length(); i += 2) {
            if (Memory::ReadU16Native(&string[i]) == 0) {
                nullTerm = i;
                break;
            }
        }
    } else {
        for (usize i = 0; i < string.Length(); ++i) {
            if (string[i] == 0) {
                nullTerm = i;
                break;
            }
        }
    }
    return nullTerm;
}

bool TagReader::ReadV2TextField(BytesMut data, String& textOut) const {
    // Text encoding    $xx
    // Information    <text string according to encoding>
    const bool isUTF16 = data[0];
    return ReadTextWithEncoding(isUTF16, data.Skip(1), textOut);
}

bool TagReader::ReadV2UserTextField(BytesMut data, String& descOut, String& valOut) const {
    // <Header for 'User defined text information frame', ID: "TXXX">
    // Text encoding    $xx
    // Description    <text string according to encoding> $00 (00)
    // Value    <text string according to encoding>
    const bool isUTF16 = data[0];

    // find terminator
    const auto nullTerm = FindNullTerminator(isUTF16, data.Skip(1));
    if (!nullTerm) {
        Debug::QError$("no null terminator in user defined text field!");
        return false;
    }

    return ReadTextWithEncoding(isUTF16, data.Subspan(1, *nullTerm), descOut)
        && ReadTextWithEncoding(isUTF16, data.Subspan(*nullTerm + 2), valOut);
}

bool TagReader::ReadV2URLField(BytesMut data, String& urlOut) const {
    // <Header for 'URL link frame', ID: "W000" - "WZZZ", excluding "WXXX" described in 4.3.2.>
    // URL <text string>
    urlOut = String((const char*)data.Data());
    return true;
}

bool TagReader::ReadV2UseURLField(BytesMut data, String& descOut, String& urlOut) const {
    // <Header for 'User defined URL link frame', ID: "WXXX">
    // Text encoding    $xx
    // Description    <text string according to encoding> $00 (00)
    // URL    <text string>
    const bool isUTF16 = data[0];

    const auto nullTerm = FindNullTerminator(isUTF16, data.Skip(1));
    if (!nullTerm) {
        Debug::QError$("no null terminator in user defined URL field!");
        return false;
    }

    return ReadTextWithEncoding(isUTF16, data.Subspan(1, *nullTerm), descOut)
        && ReadTextWithEncoding(false, data.Subspan(*nullTerm + 2), urlOut);
}

Option<ID3v2::Metadata> TagReader::ReadV2() {
    using namespace ID3v2;
    file.seekg(0, std::ios_base::beg);

    Metadata meta = {};
    if (!ReadV2Header(meta)) return nullptr;

    for (usize readSize = 0; readSize < meta.size;) {
        Tag tag = {};
        if (!ReadV2FrameHeader(tag)) return nullptr;
        if (tag.id == TagID::NONE) break; // just skip everything; we've hit padding

        meta.tags.Push(tag);
        file.seekg(tag.size, std::ios_base::cur);
        readSize += sizeof(FrameHeader) + tag.size;
    }
    return meta;
}
