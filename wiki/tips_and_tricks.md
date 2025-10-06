- 🧠 Look at [the config](core_library/4_configs)! Knowing the exact damage values is exactly the kind of thing you'd want to consider before trying out a strategy. And you don't want to be finding out there were two more units you didn't even know about two hours before the event ends!

- 💪 Consider the powerful possibilities of the `void *data` field in every object! Here, you can save any data you want, allowing you to easily execute more detailed strategies and coordinate your troops efficiently! The library wont touch it. Just remember to free everything at the end.

- ❓ If you can't find a standard library function to do what you're looking for, don't sweat it! Everything there is to know about the game at the current moment can be found in [the game struct](core_library/1_game_struct). Get any info you want yourself!

- 🫵🏻 Be careful what you free! Some standard library functions need you to free their returned array, some don't! You never need to free anything in the game struct, and you never need to free a single `t_obj *` you haven't manually allocated yourself! If you free things wrong, the library and subsequently your bot will fail pretty spectacularly pretty quickly.

- 👀 Looking at the [taxicab / manhattan distance](https://en.wikipedia.org/wiki/Taxicab_geometry) might be useful. It's what the entire project uses. Who like Pythagoras and euclidian geometry anyways? Diamonds for the win!
