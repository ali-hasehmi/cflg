## Improvements
- [x] Convert default functions(e.g. flgset_int, flgset_bool) to macros
- [ ] Create a mechanism to customize output stream of library
- [ ] Create a mechanism to customize help message
- [ ] Add usage function to print arbitrary usage message before showing help
- [ ] Refactor flgset_parse function
- [ ] Add a mechansim for flags to have aliases
- [ ] flgset_parse uses malloc for storing non-flag args, use something else instead
- [ ] print the list of matching options in the case of ambiguity

## Bugs:
- [ ] Program doesn't differentiate between '-h' and "--h"
- [x] If an arugment is forced on a bool flag(e.g. --verbose=forced_argument), program ignores the arugment instead of producing error
- [x] There is no mechanism to inform parsers if an arugment has been forced
- [x] Long flags auto completion is not working properly
- [x] Using empty string as name_long of a flag casues malformation when printing flags
