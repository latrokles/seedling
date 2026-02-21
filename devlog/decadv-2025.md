# December Adventure 2025 (?)

another attempt at the low-key December Adventure though I feel I've had way too much and have enough of an adventure on the way. still, would like to try and write a little bit of fun nonsense code whenever I manage.

## 01

some ideas I'd like to explore...

- spending some time with rust...
  - bikesheddr
  - tedit
- migrating all my code off various services into an actual self hosted server.
- spending some time learning `jj`.
- a simple language with content addressable code, codebase as database, and simple predicate dispatching ... to much?

## 02

sneaked some time to port Mary Rose Cook's Little Lisp Interpreter into python. Kinda to get back into the swing of things and to perhaps use it as the building block of some of the other things I want to experiment with.

For example:
- can i add codebase as database features ?
- can i add objects and predicate dispatching methods ?
- can i make the code content addressable ?
- can i add types using something like [this](https://jimmyhmiller.com/easiest-way-to-build-type-checker) ?

little lisp is fairly straightforward to implement (specially in python) and all so I can get to the things I want to play with fairly fast... and if time and energy permits then perhaps apply whatever is learned into some of the slightly more involve languages (that aren't lisps)

## 03

finished little lisp in python, but found it too limiting for some of the tinkering I wish to do... this is expected as it is meant to be an educational implementation. Still, I feel it can be extended further to get where I need to get in order to start so I'm debating whether to do that or start from a different foundation.

## 06 - 07

been wanting to give `rust` an honest try so felt like whatever I'm trying to do I could try and get it done with `rust`. started by porting some `c` code that implements the bluebook's `bitblt` algorithm and some graphical runtime to build stuff on top of. totally impractical, but fun.

as it's been the case the many other times I have given `rust` a try I've been flipping back and forth between "hell yea!" and "I hate this".

The SDL2 crate's API is quite different from the `c`'s and I cannot use the same old patterns, trying to separate instantiation of SDL resources and their usage was rather frustrating and confusing. I ended giving up trying to find a solution and instead put all the SDL2 resource creation and main loop in `main` and then use the instance of my custom runtime substrate alongside it (rather than embedding SDL resources into it).

Even now as I get a little bit more momentum I find myself just doing what I think the compiler wants me to do with a little help from forum threads and error message searches, but I'm certain some of this is going to come back to bite me sooner rather than later.

I will start again on the **rust book** after I get a few things here done. This feels kinda backwards, but I've always kinda learned my way through things trying them out... tho I feel rust will be the exception. Still trying to keep it fun.

I stopped short of implementing `clip` and `copy_bits`/`merge` for `bitblt`. Once that's done I can implement bresenham's and a few other graphics primitives and then port my basic doodling test program.
