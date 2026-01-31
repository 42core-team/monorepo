If you are in an event, **look at the config**! They contain invaluable info.

# Config Types

- The Game Config contains all gameplay-related settings: Units and their damage values, deposit gem counts, and bomb countdowns.
- The Server Config contains settings about how the game runs, e.g. timeouts.

The configs also contain comments detailing what each field in the config means.

# Where do I find the Config

It is visible on the website when selecting the event you are currently participating in, with some fancy visualizations.

Once you've cloned your bot, you can also find the configs in the configs/ folder.

If it's easier, you can also print the config using the provided core library functions.

> **FUN FACT**: You can totally edit the configs in your local folder, and it will work. It's a lot of fun to play around with and see how stable and reactive your bot is, but it won't have an effect on games played on the website or in the final tournament.

There are also plenty of functions to easily get the data from the configs in the Core Library - look at the reference for more infos on this.

# Config Changes

There may be config changes mid-event. This is expected and is nothing to worry about, the changes will usually be quite small. You will be alerted that a config change occurred when running the game normally.
