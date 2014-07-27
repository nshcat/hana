/*!
@file
Includes all the library components except the adapters for external
libraries.

@copyright Louis Dionne 2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 */

#ifndef BOOST_HANA_HPP
#define BOOST_HANA_HPP

//! @defgroup details Details
//! Implementation details.

//! @defgroup typeclasses Type classes
//! General purpose type classes provided by the library.
//!
//! @todo
//! - Implement better law checking and check them for all instances in the
//!   unit tests. Since we're modularized, it's OK to resort to heavy stuff
//!   in the law-checking because it's presumably only done in unit testing
//!   contexts.
//! - Provide a way to check type class requirements like
//!   `Functor f => Applicative f`.
//! - Find a better way to provide instances between type classes; consider
//!   something like `Foldable::instance<Iterable>`.
//! - Consider inheriting from a base class even when no mcd is required.
//!   That would allow us to _not_ include a useless mcd.
//! - Document the purpose of minimal instances; they are meant to provide an
//!   easy to use archetype for testing and their tests are meant to exercise
//!   the basic dispatching code of type classes (hence it makes sense to test
//!   even the mcd of a minimal instance). In particular, they are not meant
//!   to be _the_ minimal instance, which does not exist in general
//!   (what does it even mean to be a _minimal_ instance?)

//! @defgroup datatypes Data types
//! General purpose data types provided by the library.

/*!
@mainpage Boost.Hana Manual

@tableofcontents


@section preface Preface

------------------------------------------------------------------------------
The seed that became this library was planted in late 2012, when I first
started to reimplement the [Boost.MPL][] using C++11 in a project named
[MPL11][]. In spring 2014, I applied to [Google Summer of Code][GSoC] with
that project for the Boost organization and got in. The goal was to polish
the MPL11 and get it in Boost by the end of the summer. In May, before GSoC
was started full steam, I presented the project at [C++Now][] and had
insightful conversations with several attendees. The idea that it was
possible to unify the [Boost.Fusion][] and the [Boost.MPL][] libraries
made its way and I became convinced of it after writing the first prototype
for what is now Boost.Hana. We are now in late July and the plan is to
request an official review to get the library in Boost by the end of GSoC;
we'll see how that works out.

Let the fun begin.


@section introduction Introduction

------------------------------------------------------------------------------
Boost.Hana is a library of combinators tailored towards the manipulation of
heterogeneous collections. However, the core of Hana is a powerful system for
ad-hoc polymorphism inspired by Haskell type classes; this extension system
is then used to provide all the functionality of the library in a modular way.

The library uses a purely functional style of programming, which is required
to manipulate objects of heterogeneous types -- it is impossible to modify the
type of an object, so a new object must be introduced.

Contrary to previous metaprogramming libraries like Boost.MPL and Boost.Fusion,
the design of the library is not based on that of the STL. Rather, it is
strongly inspired by several (standard and non standard) modules written for
the Haskell programming language. Through experience, I have found this to
be much more expressive, flexible and easy to use while not sacrificing any
performance given the purely functional setting.


@section quick-start Quick start

------------------------------------------------------------------------------
This section assumes the reader is already familiar with `std::tuple` and
basic metaprogramming. First, let's include the library:

@dontinclude example/tutorial/quickstart.cpp
@skip boost/hana.hpp
@until using namespace

> #### Note
> Unless specified otherwise, the documentation assumes that the above lines
> are present before examples and code snippets. Use your judgment!

Finer grained headers are provided and will be explained in the
@ref organization section, but for now this will do. Let's create an
heterogeneous list, which is conceptually the same as a `std::tuple`:

@skipline auto xs =

Here, `list` is a generic lambda taking a variable number of arguments and
returning an object which is a valid sequence for Boost.Hana. The actual type
of the object returned by `list` is left unspecified, as will always be the
case in this library. However, something called its "data type" is specified;
`list` returns an object of data type `List`. Data types will be explained in
detail in their own section. There are several operations that can be performed
on lists; here are a couple so you get the feeling:

@skip assert
@until for_each

An interesting observation is that `is_empty` returns a value that can be
constexpr-converted to bool even though the list contains non-constexpr
objects (a `std::string`). Indeed, the size of the sequence is known at
compile-time regardless of its content, so it only makes sense that the
library does not throw away this information. Let's take that `for_each`
for a tour and write a function that prints a list as XML:

@skip auto to_xml =
@until to_xml(

One of the initial goals of the library was to unify type level programming
with value level programming. So in principle, it should be possible to
manipulate types and sequences of types just as one would do with the
Boost.MPL. Here is how Hana does it:

@skip auto ts =
@until fmap

There is a lot going on here. First, `type` is a variable template, and
`type<T>` is an object representing the C++ type `T`. Since it's an object,
it makes perfect sense to create a list out of these guys. Second, `fmap` is
a function similar to `std::transform`: it takes a function object and a list
(actually any `Functor` -- more on this later), applies the function to every
element in the list and returns the resulting list. Now, this means that
somehow `metafunction<std::add_pointer>` is in fact a function object.
Specifically, `metafunction` is a variable template taking a template
template parameter, and `metafunction<std::add_pointer>` is a function
object which accepts a `type<T>` and returns `type<std::add_pointer<T>::%type>`.
Still with me?

So far, we can perform computations on C++ types but we can't really do
anything useful with the result if we can't get the type out, i.e. get the
`T` out of `type<T>`. This is easily done:

@skip static_assert
@until >::value

It turns out that while `type<T>` has an unspecified C++ type, that C++ type
is still guaranteed to have a nested type named `type` which is an alias to
`T`. In Boost.MPL parlance, `decltype(type<T>)` is a nullary metafunction
returning `T`.

This is it for the quick start. Of course, there is much more available like
associative sequences, sets, ranges and even an heterogeneous `std::optional`
called `Maybe`, but you can read on if you want to know more.


@section organization Organization

------------------------------------------------------------------------------
The library is designed to be very modular while keeping the number of headers
that must be included to get basic functionality reasonably low. The structure
of the library is influenced by the structure of type classes and data types,
which will be covered later. Once you are familiar with these basic concepts,
the header structure should feel intuitive.

- `boost/hana.hpp`\n
  This is the master header of the library. It includes the whole public
  interface of the library except adapters for external libraries, which
  must be included separately.

- `boost/hana/`

  - `boost/hana/core.hpp`\n
    This file defines core utilities of the library that are tied to the
    type class dispatching and data type system.

  - `boost/hana/[typeclass].hpp`\n
    A file of this type includes the whole definition of a type class
    `[typeclass]`. This includes all the type class methods, minimal
    complete definitions and laws related to the type class.

  - `boost/hana/[typeclass]/[typeclass].hpp`\n
    A file of this type includes the definition of the type class structure
    (but not its members like `mcd`, `laws`, etc..) and the associated type
    class methods. It also defines the operators associated to its methods
    and the default-provided instances for builtin types, if there are any.
    Note that default-provided instances for non-builtin types are not
    defined in this header. This header is included by all other headers
    inside its directory.

  - `boost/hana/[typeclass]/[mcd].hpp`\n
    A file of this type defines a minimal complete definition named `[mcd]`
    along with default-provided instances for non-builtin types, if any. It
    is possible for a type class to have several minimal complete definitions,
    in which case there are several such headers with a proper name.

  - `boost/hana/[typeclass]/laws.hpp`\n
    This file defines the laws associated to a type class. Not all type
    classes have laws associated to them and some type classes have laws
    which are too hard to check, in which case this header is not provided.

  - `boost/hana/[typeclass]/instance.hpp`\n
    Some type classes are such that all instances are forced to be isomorphic.
    In that case, it sometimes makes sense to provide an implementation of the
    unique instance. When such an instance is provided, it is defined in this
    header.

  - `boost/hana/[datatype].hpp`\n
    A file of this type defines a data type named `[datatype]`. It defines
    all the type class instances associated to that data type, so that one
    only has to include the data type's header to get the full functionality
    it supports.

  - `boost/hana/ext/`\n
    This directory contains adapters for external libraries. This is the only
    part of the public interface which is not included by the master header,
    because that would make the master header dependent on those external
    libraries. Note that only the strict minimum required to adapt the
    external component is included in these headers (e.g. a forward
    declaration). This means that the definition of the external component
    should still be included when one wants to use it. For example:

    @snippet example/tutorial/include_ext.cpp main

  - `boost/hana/sandbox/`\n
    This directory contains experimental code on which no guarantee whatsoever
    is made. It might not even compile and it will definitely not be stable.

  - `boost/hana/detail/`\n
    This directory contains utilities required internally. Nothing in `detail/`
    is guaranteed to be stable, so you should not use it.

### Example
Let's say I want to include `set`. I only have to include its header and I
can use all the methods it supports right away:

@snippet example/tutorial/include_set.cpp main


@section typeclasses Type classes

------------------------------------------------------------------------------
Conceptually, type classes are an artifice allowing humans to manipulate
objects of heterogeneous types with well-defined semantics. They serve
a purpose very similar to C++ concepts (which are not in yet) and to
Haskell type classes, except they do not have language support.

To get my point through, let me make the following claim: a function template
that compiles with an argument of every possible type must have a trivial
implementation, in the sense that it must do nothing with its argument except
perhaps return it. Hence, for a function template to do something interesting,
it must fail to compile for some set of arguments. While I won't try to prove
my claim formally -- it might be false in some corner cases --, think about it
for a moment. Let's say I want to apply a function to each element of an
heterogeneous sequence:

@code{cpp}
  for_each(list(x, y, z), f)
@endcode

The first observation is that `f` must have a templated call operator because
`x`, `y` and `z` have different types. The second observation is that without
knowing anything specific about the types of `x`, `y` and `z`, it is impossible
for `f` to do anything meaningful. For example, could it print its argument?
Of course not, since it does not know whether `std::cout << x` is well-formed!
In order to do something meaningful, the function has to put constraints on
its arguments, it has to define a domain. In other words, it can't be _fully_
polymorphic, at least not conceptually. The current language does not provide
a way to express this (concepts are not there yet), so we would write something
like this instead:

@code{cpp}
  // compile-time precondition for `f(x)` to be well-formed:
  // `std::cout << x` must be a valid expression, i.e. `x` must be `Printable`
  auto f = [](auto x) {
    std::cout << x;
  };
@endcode

While the compile-time precondition is only a comment which the compiler is
not aware of, there is nonetheless a conceptual constraint on the argument
of `f`. This allows us to define the domain of `f`, which is any `Printable`
object. In straight C++, the domain of a function is a C++ type. However, it
should now be clear from the example that more flexibility could be achieved
by allowing domains to be arbitrary sets, which is roughly what C++ concepts
will bring to the table.

> Some libraries trying to make these type constraints more explicit have
> been built before. An example is [Boost.ConceptCheck][].

Type classes in Boost.Hana are a library-level implementation of such type
constraints allowing us to organize our generic programming. They allow us
to bundle together related operations (called methods), to explicitly state
that a type satisfies some constraints and how, and to do all sorts of tricks
to reduce the amount of code we have to write. However, they are in no way a
replacement for concepts; because they do not have language-level support,
the type constraints are not checked explicitly by the compiler and failure
to satisfy constraints will result in the usual compiler errors we all know
and love.


@subsection getting-concrete Getting concrete

We're now ready to take a look at these little beasts. Concretely, a type
class is just a C++ structure or class calling the `BOOST_HANA_TYPECLASS`
macro in its definition at public scope:

@code
  struct Printable {
    BOOST_HANA_TYPECLASS(Printable);
  };
@endcode

The `BOOST_HANA_TYPECLASS` macro creates a nested template named `instance`
-- which will be explained later --, so that name is reserved inside a type
class to avoid clashes. Other arbitrary members can be put in the type class,
but it is probably a good idea to keep anything unrelated out for the sake of
separating concerns:

@code
  struct Printable {
    BOOST_HANA_TYPECLASS(Printable);

    // Not invalid, but probably stupid
    int foo;
    void bar() const { };
    struct baz { };
  };
@endcode

When I introduced type classes, I said they allowed us to bundle together
related operations called methods. This is because type classes can be seen
as defining some kind of public interface consisting of the operations that
are valid with any object satisfying the constraints of the type class. I
also said that they made it possible to explicitly state that a type satisfies
a type class and how it does so. From now on, I will refer to the set of all
types satisfying a type class `T` as the _instances_ of `T`, and I will refer
to the act of specifying how a type `t` is an _instance_ of `T` as the act of
_instantiating_ `T` with `t`. Note that this has nothing to do with C++
template instantiation; it is just the standard Haskell vocabulary.

To associate methods to a type class, we create a layer of indirection through
the `instance` member of the type class. For example, let's say we want to
have a method named `print` in the `Printable` type class:

@code
  auto print = [](std::ostream& os, auto x) {
    return Printable::instance<decltype(x)>::print_impl(os, x);
  };
@endcode

> #### Note
> This is actually _slightly_ different from what is really done in the
> library; this will be clarified in the section on data types.

To make a type an instance of `Printable`, we must implement the `print`
method, which is done by specializing the `Printable::instance` template
as follows:

@code
  template <>
  struct Printable::instance<int> {
    static void print_impl(std::ostream& os, int i) {
      os << i;
    }
  };
@endcode

Note that we could have chosen a name different from `print_impl`, but this
naming convention has the advantage of being clear and avoiding name
clashes inside the instance. In particular, we would not want to use
`Printable::instance<int>::%print`, because using `print` inside the
instance would then refer to the implementation instead of `::%print`,
which is unexpected. This naming convention is used for all the type
classes in Boost.Hana. Now that we have made `int` an instance of
`Printable`, we can write:

@code
  print(std::cout, 2);
@endcode

So far so good, but you probably don't want to write an instance for each
arithmetic type, right?. Fortunately, I didn't want to either so it is
possible to instantiate a type class for all types satisfying a predicate:

@code
  template <typename T>
  struct Printable::instance<T, when<std::is_arithmetic<T>::value>> {
    static void print_impl(std::ostream& os, T x) {
      os << x;
    }
  };
@endcode

`when` accepts a single compile-time boolean and enables the instance if and
only if that boolean is `true`. This is similar to the well known C++ idiom of
using a dummy template parameter with `std::enable_if` and relying on SFINAE.
As expected, we can now write

@code
  print(std::cout, 2.2);
@endcode

Ok, we managed to cut down the number of instances quite a bit, but we still
can't write

@code
  print(std::cout, std::string{"foo"});
@endcode

without writing an explicit instance for `std::string`. Again, laziness won me
over and so it is possible to instantiate a type class for all types making
some expression well-formed (think SFINAE):

@code
  template <typename T>
  struct Printable::instance<T, when_valid<
    decltype(std::declval<std::ostream&>() << std::declval<T>())
  >> {
    static void print_impl(std::ostream& os, T x) {
      os << x;
    }
  };
@endcode

`when_valid` is actually an alias to `when<true>`, but it takes an arbitrary
number of types and relies on the fact that SFINAE will kick in and remove
the specialization if any of the types is not well-formed. As expected, we
can now write

@code
  print(std::cout, std::string{"foo"});
@endcode

Note that instances provided without `when`/`when_valid` (i.e. an explicit
or partial specialization in the case of a parametric type) have the priority
over instances provided with it. This is to allow types to instantiate a type
class even if an instance for the same type class is provided through a
predicate. This design choice was made assuming that a specialization
(even partial) is usually meant to be more specific than a catch-all
instance enabled by a predicate.

All is good so far, but what if we just wanted a string representation of
our object instead of printing it to a stream? Sure, we could write

@code
  auto to_string = [](auto x) {
    std::ostringstream os;
    print(os, x);
    return os.str();
  };

  to_string(1);
@endcode

but then `to_string(std::string{"foobar"})` would be far from optimal. The
solution is to make `to_string` a method too, and then specialize the
instance for `std::string` to make it more efficient:

@code
  auto to_string = [](auto x) {
    return Printable::instance<decltype(x)>::to_string_impl(x);
  };

  template <typename T>
  struct Printable::instance<T, when_valid<
    decltype(std::declval<std::ostream&>() << std::declval<T>())
  >> {
    static void print_impl(std::ostream& os, T x) {
      os << x;
    }

    static std::string to_string_impl(T x) {
      std::ostringstream os;
      print(os, x);
      return os.str();
    }
  };

  template <>
  struct Printable::instance<std::string> {
    static void print_impl(std::ostream& os, std::string x) {
      os << x;
    }

    static std::string to_string_impl(std::string x)
    { return x; }
  };
@endcode

While this is pretty satisfying, notice how the general definition of
`to_string_impl` is tied to the instance it is defined in, even though it
would work for any `Printable` implementing `print`. For example, let's say
we want to instantiate `Printable` for `std::vector<T>` for any `T` which can
be put to a stream:

@code
  template <typename T>
  struct Printable::instance<std::vector<T>, when_valid<
    decltype(std::declval<std::ostream&>() << std::declval<T>())
  >> {
    static void print_impl(std::ostream& os, std::vector<T> v) {
      os << '[';
      for (auto it = begin(v); it != end(v); ) {
        os << *it;
        if (++it != end(v))
          os << ", ";
      }
      os << ']';
    }

    static std::string to_string_impl(std::vector<T> v) {
      std::ostringstream os;
      print(os, v);
      return os.str();
    }
  };
@endcode

It is annoying to have to redefine `to_string_impl` even if `print_impl` alone
would be enough because `to_string_impl` is completely determined by it. From
now on, I will refer to the minimal set of methods that are required to
instantiate a type class as the minimal complete definition, abbreviated
`mcd`. In the present case, the minimal complete definition for the `Printable`
type class is `print`. To avoid code duplication, a default implementation for
methods that are not part of the mcd should be provided by the type class. By
convention, we provide these in a nested type of the type class:

@code
  struct Printable {
    BOOST_HANA_TYPECLASS(Printable);

    struct mcd {
      template <typename X>
      static std::string to_string_impl(X x) {
        std::ostringstream os;
        print(os, x);
        return os.str();
      }
    };
  };
@endcode

It then suffices to inherit from that structure when instantiating `Printable`
to get the default implementation for `to_string_impl`, which can now be
shared by several instances:

@code
  template <typename T>
  struct Printable::instance<T, when_valid<
    decltype(std::declval<std::ostream&>() << std::declval<T>())
  >> : Printable::mcd {
    // print_impl omitted
  };

  template <typename T>
  struct Printable::instance<std::vector<T>, when_valid<
    decltype(std::declval<std::ostream&>() << std::declval<T>())
  >> : Printable::mcd {
    // print_impl omitted
  };
@endcode

> #### Note
> For simplicity, the term minimal complete definition can refer either to
> a minimal set of required methods or to the member of the type class
> providing the corresponding default implementations.

It is possible for a type class to have several minimal complete definitions.
For example, one could observe that `print` can also be implemented in terms
of `to_string`. If we wanted to do so, we could provide both minimal complete
definitions by putting them into suitably named members of the `Printable`
type class:

@code
  struct Printable {
    BOOST_HANA_TYPECLASS(Printable);

    // requires to_string only
    struct to_string_mcd {
      template <typename X>
      static void print_impl(std::ostream& os, X x) {
        os << to_string(x);
      }
    };

    // requires print only
    struct print_mcd {
      template <typename X>
      static std::string to_string_impl(X x) {
        std::ostringstream os;
        print(os, x);
        return os.str();
      }
    };
  };
@endcode

Either minimal complete definition could now be used to instantiate `Printable`.
By convention, in Boost.Hana, the minimal complete definition is always named
`mcd` when there is a single one. A nested type named `mcd` is also provided
when there are no default implementations to provide for consistency and for
extensibility, as will be explained next. If there is more than one possible
mcd, each mcd is in a different nested type with a descriptive name. In all
cases, the minimal complete definition(s) are documented.

It is recommended to always inherit from a minimal complete definition, even
when the default implementations are not actually used:

@code
  template <>
  struct Printable::instance<std::string>
    : Printable::print_mcd // could also be to_string_mcd, it doesn't matter
  {
    static void print_impl(std::ostream& os, std::string x)
    { os << x; }

    static std::string to_string_impl(std::string x)
    { return x; }
  };
@endcode

This allows methods to be added to the type class without breaking the
instance, provided the type class does not change its minimal complete
definition(s). This is the reason why a minimal complete definition is
always provided, even when it contains no default implementations.

To show the full power of type classes and introduce the `instantiates`
utility, let's define a `Printable` instance for `std::vector`s containing
any `Printable` type. This is a generalization of our previous `Printable`
instance for `std::vector`s, which supported only streamable types.

@code
  template <typename T>
  struct Printable::instance<std::vector<T>, when<instantiates<Printable, T>()>>
    : Printable::print_mcd
  {
    static void print_impl(std::ostream& os, std::vector<T> v) {
      os << '[';
      for (auto it = begin(v); it != end(v); ) {
        print(os, *it); // recursively print the contents
        if (++it != end(v))
          os << ", ";
      }
      os << ']';
    }
  };
@endcode

`instantiates` is a variable template taking a type class and several types
and returns whether the type class is instantiated for the given types. The
result is returned as a boolean `Integral`, which is basically equivalent to
a boolean `std::integral_constant`, hence the trailing `()`. We can now print
nested containers:

@code
  std::vector<std::vector<int>> v{{1, 2, 3}, {3, 4, 5}};
  print(std::cout, v); // prints "[[1, 2, 3], [3, 4, 5]]"
@endcode

One last thing which can be done with type classes is to provide a default
instance for all data types. To do so, a nested `default_instance` template
must be defined in the type class:

@code
  struct Printable {
    BOOST_HANA_TYPECLASS(Printable);

    // definitions omitted
    struct to_string_mcd { };
    struct print_mcd { };

    template <typename T>
    struct default_instance : to_string_mcd {
      static std::string to_string_impl(T) {
        return "<not-printable>";
      }
    };
  };
@endcode

`default_instance` should be just like a normal instance. Note that this
feature should seldom be used because methods with a meaningful behavior
for all data types are rare. This feature is provided for flexibility,
but it should be a hint to reconsider your type class design if you
are about to use it.


### Example of a type class definition
@include example/core/typeclass.cpp

### Example of a type class with a default instance
@include example/core/default_instance.cpp

@todo
Document type classes with operators.


@section datatypes Data types

------------------------------------------------------------------------------
Data types are a generalization of usual C++ types making it easier to
instantiate type classes for heterogeneous containers. They are very similar
to Boost.Fusion and Boost.MPL tags. So far, we have only instantiated type
classes with types that "contained" homogeneous objects (`std::vector<T>`,
`int`, etc..). What if we wanted to make `boost::fusion::vector` an instance
of `Printable`?

@code
  template <typename ...T>
  struct Printable::instance<boost::fusion::vector<T...>>
    : Printable::print_mcd
  {
    // print_impl omitted
  };
@endcode

If you know Boost.Fusion, then you probably know that it won't work. When
we instantiate `Printable` for `boost::fusion::vector<T...>`, we're only
instantiating it for that particular representation of a Boost.Fusion vector.
This is because Boost.Fusion vectors exist in numbered forms, which are of
different types:

@code
  boost::fusion::vector1<T>
  boost::fusion::vector2<T, U>
  boost::fusion::vector3<T, U, V>
  ...
@endcode

This is an implementation detail required by the lack of variadic templates in
C++03 that leaks into the interface. This is unfortunate, but we need a way to
work around it. And it gets worse; I said earlier that `list`, `type` and
other Boost.Hana components did not specify their type at all -- specifying
those types would be too restrictive for the implementation --, so it would
be straight impossible to instantiate type classes with those. The solution
is to bundle all the types representing the "same thing" under a single tag,
and then to use that tag to perform the method dispatching. For this, we
introduce the `datatype` metafunction, which associates a "tag" to a group
of types. The `Printable` methods now become:

@code
  auto print = [](std::ostream& os, auto x) {
    return Printable::instance<
      typename datatype<decltype(x)>::type
    >::print_impl(os, x);
  };

  auto to_string = [](auto x) {
    return Printable::instance<
      typename datatype<decltype(x)>::type
    >::to_string_impl(x);
  };
@endcode

> #### Note
> Actually, it would be possible to work around the issue regarding Fusion
> vectors by using `boost::fusion::sequence_tag` with `when` to instantiate
> `Printable` only when it is a Fusion vector, but it still does not resolve
> the issue for Hana components that do not specify anything about their type.

By default, `datatype<T>::%type` is `T::hana_datatype` if that expression
is well-formed, and `T` otherwise. It can also be specialized to allow
customizing the data type of `T` in a ad-hoc manner. Finally, a dummy
template parameter is provided to allow SFINAE-based specialization.
For `boost::fusion::vector`, we would then do:

@code
  struct BoostFusionVector;

  template <typename T>
  struct datatype<T, std::enable_if_t<
    is_a_boost_fusion_vector<T>::value
  >> {
    using type = BoostFusionVector;
  };
@endcode

Where `is_a_boost_fusion_vector` is some metafunction returning whether a
type is a Fusion vector. Hana components either provide a similar
specialization or use the nested `hana_datatype` alias to provide a data
type, which is specified. To make `boost::fusion::vector` `Printable`,
we would now write:

@code
  template <>
  struct Printable::instance<BoostFusionVector> : Printable::print_mcd {
    // print_impl omitted
  };
@endcode

The fact that `datatype<T>::%type` defaults to `T` is _very_ useful, because
it means that you can ignore it completely if the component you're building
has a well-defined type, and everything will "just work".


@todo
- When we have functions with several variants, consider naming versions
  with the following scheme:
  @code
    foldr.lazy
    foldr.strict
    foldr == foldr.strict
  @endcode
  That would be prettier and not much more complicated. However, we would
  need a naming convention for the `_impl` versions of those.
- To consider: is it possible/desirable to eliminate partial functions?
  For example, removing `head` and `tail`, which can fail, and replace
  them by a function which returns a `Maybe`.
- Document the library's stance on perfect forwarding and move semantics.
  Make compile-time tests with non-copyable types and runtime tests with
  expensive-to-copy types.
- In the unit tests, we might want to use an injective function on
  `Comparable`s instead of `std::make_tuple`.
- Document how to emulate `make_fused` and friends from Boost.Fusion.
- Document how to write common Boost.Fusion and Boost.MPL idioms with
  Boost.Hana.
- Write runtime benchmarks.
- Setup a BJam build system.
- Consider making function objects automatically curriable. This could allow
  _super sexy_ stuff like:
  @code
    template <>
    struct Iterable<List> {
        static constexpr auto length_impl = foldl(some_lambda, size_t<0>);
    };
  @endcode


<!-- Links -->
[Boost.ConceptCheck]: http://www.boost.org/doc/libs/release/libs/concept_check/index.html
[Boost.Fusion]: http://www.boost.org/doc/libs/release/libs/fusion/doc/html/index.html
[Boost.MPL]: http://www.boost.org/doc/libs/release/libs/mpl/doc/index.html
[C++Now]: http://cppnow.org
[GSoC]: http://www.google-melange.com/gsoc/homepage/google/gsoc2014
[MPL11]: http://github.com/ldionne/mpl11

 */


#include <boost/hana/applicative.hpp>
#include <boost/hana/bool.hpp>
#include <boost/hana/comparable.hpp>
#include <boost/hana/constant.hpp>
#include <boost/hana/core.hpp>
#include <boost/hana/foldable.hpp>
#include <boost/hana/functional.hpp>
#include <boost/hana/functor.hpp>
#include <boost/hana/integer_list.hpp>
#include <boost/hana/integral.hpp>
#include <boost/hana/iterable.hpp>
#include <boost/hana/lazy.hpp>
#include <boost/hana/list.hpp>
#include <boost/hana/logical.hpp>
#include <boost/hana/map.hpp>
#include <boost/hana/maybe.hpp>
#include <boost/hana/monad.hpp>
#include <boost/hana/orderable.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/range.hpp>
#include <boost/hana/record.hpp>
#include <boost/hana/searchable.hpp>
#include <boost/hana/set.hpp>
#include <boost/hana/traversable.hpp>
#include <boost/hana/type.hpp>
#include <boost/hana/type_list.hpp>

#endif // !BOOST_HANA_HPP
