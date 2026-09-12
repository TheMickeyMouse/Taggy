#include <iostream>

#include "TagReader.h"
#include "Utils/Array.h"
#include "Utils/Debug/Logger.h"
#include "Utils/Iter/Zip.h"

using namespace Quasi;

namespace CLI {
    int Run(int argc, char *argv[]);

    String ParseStringInput(Str string) {
        return string.Trim('"').Unescape().Assert("bad string input!");
    }

    int Help() {
        Text::PrintLn(
            "taggy version 1.0.0, build from https://github.com/TheMickeyMouse/Taggy\n"
            "mp3 tag extractor and editor cli tool\n"
            "usage: taggy [input-file] [options] [-o [output-file]]\n"
            "   -h, --help            Display the help screen\n"
            "   -o                    Writes the destination file to the path specified after\n"
            "   --artist=<name>       Sets the artist name\n"
            "   --title=<name>        Sets the title\n"
            "   --album=<name>        Sets the album name\n"
            "   --albart=<name>       Sets the album artist name\n"
            "   --disc=<number>       Sets the disc number\n"
            "   --track=<number>      Sets the track number\n"
            "   --date=<date>         Sets the release date, format is YYYY-MM-DD\n"
            "   --explicit            Adds the explicit tag\n"
            "   --lyrics=<lrcfile>    Sets the (unsycned) lyrics from a file source\n"
            "   --slyrics=<lrcfile>   Sets the (sycned) lyrics from a file source\n"
            "   --cover=<imagefile>   Sets the images from a file source\n"
            "   -T...=<value>         Adds a custom text field tag\n"
        );
        return 0;
    }

    int ReadMp3Tags(CStr path) {
        TagReader tagReader = { path.Data() };
        if (const auto m = tagReader.ReadV1(); m) {
            Debug::QWarn$("ID3v1 tag exists");
        }
        if (const auto m = tagReader.ReadV2(); m) {
            const auto& metadata = *m;
            Text::Print("Found tag: ID3v2.{}.{}; {} bytes in total", metadata.tagVersion >> 8, metadata.tagVersion & 0xFF, metadata.size);
            for (const auto& [tag, data] : Iter::Zip(metadata.tags.Iter(), metadata.tagFields.Iter())) {
                Text::Print("\n\t{:<16} ({} bytes): ", ID3v2::GetTagRules(tag.id).properName, tag.size);
                PrintField(data);
            }

            tagReader.file.seekg(0, std::ios::end);
            Text::PrintLn("\nTotal file size: {} bytes", std::filesystem::file_size(path.Data()));
            Text::PrintLn("Statistics:\n\tMPEG Version {} Layer {}\n\tBitrate: {}kbps, Sampling rate: {}Hz, Channels: {}",
                (Array {{ "_", "1", "2", "2.5" }})[metadata.mpgVersion],
                (Array {{ "_", "I", "II", "III" }})[metadata.layerVersion],
                metadata.bitrateKbps,
                metadata.samplingRate,
                (int)metadata.channels
            );
            return 0;
        } else {
            Text::PrintLn("Tag not found!");
            return 1;
        }
    }

    int WriteMp3Tags(CStr ipath, CStr opath, Span<const ID3v2::TagFieldPair> fields) {
        TagWriter tagWriter = { ipath.Data(), opath.Data() };
        return tagWriter.WriteV2(fields) ? 0 : 1;
    }

    int Run(int argc, char *argv[]) {
        using namespace ID3v2;
       if (argc < 2) {
            Debug::QError$("no file provided! -h for help.");
        }

        String inputFile = ParseStringInput(argv[1]), outputFile;
        CStr inputFilePath = inputFile.IntoCStr(), outputFilePath;

        if (argc == 2) { // just write out mp3 contents
            if (Str(argv[1]) == "-h") return Help();
            return ReadMp3Tags(inputFilePath);
        }

        Vec<TagFieldPair> fields;
        for (u32 i = 2; i < argc; ++i) {
            Str option = argv[i];
            if (option == "-o") {
                if (i + 1 >= argc) {
                    Debug::QError$("-o specified but no output file path after it! -h for help");
                    return 1;
                }
                outputFile = ParseStringInput(argv[++i]);
                outputFilePath = outputFile.IntoCStr();
            } else if (option == "-h") {
                return Help();
            } else if (option.StartsWith("-T")) {
                const auto [desc, value] = option.RemovePrefix("-T").SplitOnce('=');
                fields.Push(Tags::Custom(desc, ParseStringInput(value)));
            } else if (option.StartsWith("--date=")) {
                String dateString = ParseStringInput(option.RemovePrefix("--date="));
                const auto [year, date] = dateString.SplitOnce('-');
                char mmdd[4] = { date[0], date[1], date[3], date[4] }; // skip '-'
                fields.Push(Tags::Year(year));
                fields.Push(Tags::Date(Str::Slice(mmdd, 4)));
            } else if (option.StartsWith("--artist=")) {
                fields.Push(Tags::Artist(ParseStringInput(option.RemovePrefix("--artist="))));
            } else if (option.StartsWith("--album=")) {
                fields.Push(Tags::Album(ParseStringInput(option.RemovePrefix("--album="))));
            } else if (option.StartsWith("--title=")) {
                fields.Push(Tags::Title(ParseStringInput(option.RemovePrefix("--title="))));
            } else if (option.StartsWith("--disc=")) {
                fields.Push(Tags::Disc(ParseStringInput(option.RemovePrefix("--disc="))));
            } else if (option.StartsWith("--track=")) {
                fields.Push(Tags::Track(ParseStringInput(option.RemovePrefix("--track="))));
            } else if (option.StartsWith("--albart=")) {
                fields.Push(Tags::AlbumArtist(ParseStringInput(option.RemovePrefix("--albart="))));
            } else if (option.StartsWith("--lyrics=")) {
                String lyricsPath = ParseStringInput(option.RemovePrefix("--lyrics="));
                Option<String> lyrics = Text::ReadFile(lyricsPath.IntoCStr());
                if (!lyrics) {
                    Debug::QError$("couldn't find lyrics file {}!", lyricsPath);
                    return 1;
                }

                fields.Push(Tags::UsLyrics(*lyrics));
            } else if (option.StartsWith("--slyrics=")) {
                String lyricsPath = ParseStringInput(option.RemovePrefix("--slyrics="));
                Option<String> lyrics = Text::ReadFile(lyricsPath.IntoCStr());
                if (!lyrics) {
                    Debug::QError$("couldn't find lyrics file {}!", lyricsPath);
                    return 1;
                }

                fields.Push(Tags::SyLyrics(*lyrics));
            } else if (option.StartsWith("--cover=")) {
                String coverPath = ParseStringInput(option.RemovePrefix("--cover="));
                Option<String> cover = Text::ReadFileBinary(coverPath.IntoCStr());
                if (!cover) {
                    Debug::QError$("couldn't find cover image file {}!", coverPath);
                    return 1;
                }

                fields.Push(Tags::Cover(cover->IntoBytes()));
            } else if (option == "--explicit") {
                fields.Push(Tags::Explicit());
            } else {
                Debug::QError$("unrecognized option '{}', aborting! -h for help", option);
                return 1;
            }
        }

        return WriteMp3Tags(inputFilePath, outputFilePath, fields);
    }
}
