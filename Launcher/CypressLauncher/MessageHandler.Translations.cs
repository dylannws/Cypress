#nullable enable
using System;
using System.IO;
using System.Linq;
using Newtonsoft.Json.Linq;

namespace CypressLauncher;

public partial class MessageHandler
{
    private static string GetTranslationsDir() =>
        Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "Cypress", "Translations");

    private void OnGetTranslations(string lang)
    {
        if (string.IsNullOrWhiteSpace(lang) || !System.Text.RegularExpressions.Regex.IsMatch(lang, @"^[a-zA-Z0-9_\-]{1,32}$"))
            lang = "en_us";

        string appdataDir = GetTranslationsDir();
        string appdataFile = Path.Combine(appdataDir, lang + ".json");
        string bundledFile = Path.Combine(AppContext.BaseDirectory, "assets", "translations", lang + ".json");

        JObject? strings = null;

        if (File.Exists(bundledFile))
        {
            try { strings = JObject.Parse(File.ReadAllText(bundledFile)); } catch { }

            // overwrite AppData so users always get the latest bundled translations
            try
            {
                Directory.CreateDirectory(appdataDir);
                File.Copy(bundledFile, appdataFile, overwrite: true);
            }
            catch { }
        }
        else if (File.Exists(appdataFile))
        {
            // user-provided translation with no bundled equivalent
            try { strings = JObject.Parse(File.ReadAllText(appdataFile)); } catch { }
        }

        if (strings == null)
        {
            if (lang != "en_us") { OnGetTranslations("en_us"); return; }
            Send(new JObject { ["type"] = "translations", ["lang"] = "en_us", ["strings"] = new JObject() });
            return;
        }

        // merge en_us as fallback for any keys missing from this language
        if (lang != "en_us")
        {
            string enFile = Path.Combine(AppContext.BaseDirectory, "assets", "translations", "en_us.json");
            if (File.Exists(enFile))
            {
                try
                {
                    var en = JObject.Parse(File.ReadAllText(enFile));
                    foreach (var prop in en.Properties())
                    {
                        if (strings[prop.Name] == null)
                            strings[prop.Name] = prop.Value;
                    }
                }
                catch { }
            }
        }

        Send(new JObject { ["type"] = "translations", ["lang"] = lang, ["strings"] = strings });
    }

    private void OnGetTranslationsList()
    {
        var langs = new System.Collections.Generic.HashSet<string>();

        string bundledDir = Path.Combine(AppContext.BaseDirectory, "assets", "translations");
        if (Directory.Exists(bundledDir))
        {
            foreach (string f in Directory.GetFiles(bundledDir, "*.json"))
                langs.Add(Path.GetFileNameWithoutExtension(f));
        }

        string appdataDir = GetTranslationsDir();
        if (Directory.Exists(appdataDir))
        {
            foreach (string f in Directory.GetFiles(appdataDir, "*.json"))
                langs.Add(Path.GetFileNameWithoutExtension(f));
        }

        Send(new JObject { ["type"] = "translationsList", ["langs"] = new JArray(langs.OrderBy(x => x)) });
    }
}
