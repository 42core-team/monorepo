Files in `bots/base` are shared across every discovered language and sync into its `bot` target.
The sync only manages files that exist in `bots/base`, and it never deletes unmanaged files from targets.
Run `make -C bots sync-base` to apply the sync, or `make -C bots check-base` to verify that managed files are in sync.
Text files may use `{{LANG}}`, which is replaced with the language slug during sync.
