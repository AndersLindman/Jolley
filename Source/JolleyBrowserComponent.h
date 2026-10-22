#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <BinaryData.h>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>

class JolleyBrowserComponent : public juce::WebBrowserComponent
{
public:
    explicit JolleyBrowserComponent (const Options& options)
        : juce::WebBrowserComponent (options)
    {
    }

    static Options createDefaultOptions()
    {
        return Options {}
            .withBackend (Options::Backend::webview2) 
            .withWinWebView2Options (
                Options::WinWebView2 {}
                    .withBackgroundColour (juce::Colour::fromRGB (12, 12, 12))
                    .withUserDataFolder (juce::File::getSpecialLocation (juce::File::SpecialLocationType::tempDirectory))
            )
            .withNativeIntegrationEnabled (true)
            .withResourceProvider ([] (const juce::String& url) -> std::optional<Resource> {
                return getResource (url);
            }, "http://127.0.0.1:5173"); // allowedOrigin: lets the Vite dev server fetch from the resource provider
    }

    // Helper to safely extract the file path from any URL format
    static juce::String getPathFromUrl (const juce::String& url)
    {
        juce::String path = url;

        // 1. Strip scheme and host if present (e.g., "https://juce.backend/src/main.js")
        if (path.startsWith ("http://") || path.startsWith ("https://"))
        {
            const int schemeEnd = path.indexOf ("://") + 3;
            const int pathStart = path.indexOfChar (schemeEnd, '/');
            path = pathStart >= 0 ? path.substring (pathStart) : "/";
        }

        // 2. Strip leading slashes
        while (path.startsWithChar ('/'))
            path = path.substring (1);

        // 3. Strip query strings or fragments (e.g., "?v=1.2" or "#section")
        if (const int q = path.indexOfChar ('?'); q >= 0) path = path.substring (0, q);
        if (const int h = path.indexOfChar ('#'); h >= 0) path = path.substring (0, h);

        // 4. Normalize backslashes to forward slashes (handles Windows ZIP quirks)
        path = path.replace ("\\", "/");

        // 5. Default to index.html if empty
        return path.isEmpty() ? juce::String ("index.html") : path;
    }

    static std::optional<Resource> getResource (const juce::String& url)
    {
        auto urlToRetrieve = getPathFromUrl (url);

        if (auto* archive = getZipFile())
        {
            // Try multiple path formats to handle Windows ZIP quirks robustly
            juce::StringArray pathsToTry;
            pathsToTry.add (urlToRetrieve);                                    // src/main.js
            pathsToTry.add (urlToRetrieve.replace ("/", "\\"));                 // src\main.js
            pathsToTry.add ("ui/" + urlToRetrieve);                            // ui/src/main.js
            pathsToTry.add ("ui/" + urlToRetrieve.replace ("/", "\\"));        // ui\src\main.js
            
            const juce::ZipFile::ZipEntry* entry = nullptr;
            
            for (const auto& path : pathsToTry)
            {
                entry = archive->getEntry (path);
                if (entry != nullptr)
                    break;
            }

            if (entry != nullptr)
            {
                auto stream = juce::rawToUniquePtr (archive->createStreamForEntry (*entry));
                if (stream != nullptr)
                {
                    auto v = streamToVector (*stream);
                    auto mime = getMimeForExtension (getExtension (entry->filename).toLowerCase());
                    return Resource { std::move (v), std::move (mime) };
                }
            }
        }

        // Only show fallback if the root HTML itself is missing
        if (urlToRetrieve == "index.html")
        {
            static constexpr char fallbackHtml[] = 
                "<html><body style='background:#121212;color:#fff;font-family:sans-serif;display:flex;justify-content:center;align-items:center;height:100vh;margin:0;'>"
                "<h2>Jolley UI Bundle Missing</h2><p>Please check that jolley-gui.zip is bundled in BinaryData.</p>"
                "</body></html>";
            
            juce::MemoryInputStream fallbackStream { fallbackHtml, sizeof (fallbackHtml) - 1, false };
            return Resource { streamToVector (fallbackStream), juce::String { "text/html" } };
        }

        return std::nullopt;
    }

    bool pageAboutToLoad (const juce::String& newURL) override
    {
        // Allow about:blank (WebView2 sometimes requests this internally)
        if (newURL == "about:blank")
            return true;

        auto root = getResourceProviderRoot();
        return newURL.startsWith ("http://localhost")
            || newURL.startsWith ("http://127.0.0.1")
            || (root.isNotEmpty() && newURL.startsWith (root));
    }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JolleyBrowserComponent)

private:
    static juce::ZipFile* getZipFile()
    {
        static std::unique_ptr<juce::MemoryInputStream> zipStream;
        static std::unique_ptr<juce::ZipFile> archive;
        static bool initialised = false;

        if (!initialised)
        {
            initialised = true;
            
            // Use the EXACT variable names generated by CMake in BinaryData.h
            int dataSize = BinaryData::jolleygui_zipSize;
            const char* data = BinaryData::jolleygui_zip;

            if (data != nullptr && dataSize > 0)
            {
                // 1. Create the stream
                zipStream = std::make_unique<juce::MemoryInputStream> (data, static_cast<size_t> (dataSize), false);
                
                // 2. Pass the POINTER (.get()) to the ZipFile
                archive = std::make_unique<juce::ZipFile> (zipStream.get(), false);
            }
        }

        return archive.get();
    }

    static const char* getMimeForExtension (const juce::String& extension)
    {
        static const std::unordered_map<juce::String, const char*> mimeMap =
        {
            { "htm",   "text/html"                },
            { "html",  "text/html"                },
            { "txt",   "text/plain"               },
            { "jpg",   "image/jpeg"               },
            { "jpeg",  "image/jpeg"               },
            { "svg",   "image/svg+xml"            },
            { "ico",   "image/vnd.microsoft.icon" },
            { "json",  "application/json"         },
            { "png",   "image/png"                },
            { "css",   "text/css"                 },
            { "map",   "application/json"         },
            { "js",    "application/javascript"   },
            { "mjs",   "application/javascript"   },
            { "woff2", "font/woff2"               },
            { "woff",  "font/woff"                },
            { "ttf",   "font/ttf"                 }
        };

        if (const auto it = mimeMap.find (extension); it != mimeMap.end())
            return it->second;

        return "application/octet-stream";
    }

    static juce::String getExtension (const juce::String& filename)
    {
        return filename.fromLastOccurrenceOf (".", false, false);
    }

    static std::vector<std::byte> streamToVector (juce::InputStream& stream)
    {
        std::vector<std::byte> result ((size_t) stream.getTotalLength());
        stream.setPosition (0);
        
        // Cast to int to fix signed/unsigned mismatch warning with juce::InputStream::read
        [[maybe_unused]] const auto bytesRead = stream.read (result.data(), (int) result.size());
        jassert (bytesRead == (int) result.size());
        return result;
    }
};