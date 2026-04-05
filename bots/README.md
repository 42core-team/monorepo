Files in `bots/base` are shared across every discovered language and sync into each existing `softcore` or `hardcore` target.  
Files in `bots/<lang>/base` apply only to that language and override `bots/base` when the relative path matches.  
The sync only manages files that exist in one of those base layers, and it never deletes unmanaged files from targets.  
Run `make -C bots sync-base` to apply the layered sync, or `make -C bots check-base` to verify that managed files are in sync.  
Text files may use `{{LANG}}`, which is replaced with the language slug during sync.
