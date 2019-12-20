# MPHF
Minimal Perfect Hash Functions using SAT


# Description

This project supports building small minimal perfect hash functions
very near the entropy limit by using Satisfiability technology.

An MPHF is a bijective function that maps a set of keywords `W = {w_0,
..., w_{n-1}}` to the integers `{0, ..., n-1}`. Details of the SAT
encoding used here are presented in this paper: [Constructing Minimal
Perfect Hash Functions Using SAT
Technology](https://www.cs.cmu.edu/~mheule/publications/AAAI-WeaverS.1625.pdf).


# Dependencies

This project relies on a git submodule. To get this module, clone
the repository by doing either
```
git clone --recursive git@github.com:weaversa/MPHF.git
```
or
```
git clone git@github.com:weaversa/MPHF.git
cd MPHF
git submodule update --init --recursive
```

This project also needs a SAT solver to call out to. Any that support
the SAT competition interface should do
(http://www.satcompetition.org/). On such solver is Glucose, available
here: https://www.labri.fr/perso/lsimon/glucose/. Just make sure to
have the solver binary in your system path and set the `solver_string`
parameter appropriately (described below).


# Install

Run `make` in the project root directory. The library file
`libmphfsat.a` will (assuming successful compilation) be
created in this package's `lib` directory


# Building an MPFH

An MPHF is built in two separate phases. The first involves adding
elements to a builder object. The second involves creating a querier
object from a builder object. Once completed, the querier object may
be queried ad infinitum.

A builder is first allocated using `MPHFBuilderAlloc`, like so:

```
MPHFBuilder *mphfb = MPHFBuilderAlloc(0);
```

Here, the first and only argument `0` is the number of expected
elements the MPHF will encode. It is safe to leave this number as `0`,
but will decrease calls to malloc if the actual number is given ahead
of time.

Elements are added to the builder, like so:

```
if(MPHFBuilderAddElement(mphfb, pElement, nElementBytes) != 0) {
  fprintf(stderr, "Element insertion failed...exiting\n");
  return -1;
}
```

Here, `pElement` is a pointer to at least `nElementBytes` number of
bytes. This element will be copied into the builder.

If an element has already been hashed, the hash can be directly added to the builder, like so:

```
if(MPHFBuilderAddHash(mphfb, (MPHFHash) {.h1 = hash}) != 0) {
  fprintf(stderr, "Hash insertion failed...exiting\n");
  return -1;
}
```

After all elements (or hashes) have been stored, the querier is ready
to be created:

```
MPHFQuerier *mphfq = MPHFBuilderFinalize(mphfb, test_parameters);
```

The first argument is the builder. The second argument is a structure
consisting of two parameters: the target bits-per-element of the
resulting MPHF and the command-line string used to call a SAT
solver. For example,

```
MPHFParameters test_parameters =
  { .fBitsPerElement = 1.30,
    .solver_string = "glucose -model"
  };
```

Feel free to define your own parameters to meet the needs of your
application.

The returned querier (`mphfq`) will be `NULL` on error.

When finalizing, you will notice that some progress is printed to
stderr. These print statements can be turned off by commenting out the
following line in mphf.h and recompiling the package.

```
#define MPHF_PRINT_BUILD_PROCESS
```

After creating the querier, it is suggested that the builder be
free'd, like so:

```
MPHFBuilderFree(mphfb);
```


# Querying an MPFH

The MPHF can be queried against an element, like so:

```
uint32_t key = MPHFQuery(mphfq, pElement, nElementBytes);
```

Here, `pElement` is a pointer to `nElementBytes` number of bytes. The key unique to this element is returned.

The MPHF can be queried against an already hashed element, like so:

```
uint32_t key = MPHFQueryHash(mphfq, (MPHFHash) {.h1 = hash});
```

When querying is finished, the querier can be freed, like so:

```
  MPHFQuerierFree(mphfq);
```


# Serialization

Queriers can be serialized (written to a file) in the following way:

```
  FILE *fout = fopen("mphf.out", "w");
  if(MPHFSerialize(fout, mphfq) != 0) {
    fprintf(stderr, "Serialization failed...exiting\n");
    return -1;
  }
  fclose(fout);
```

Here, `fout` is of type `FILE *`. `ret` will be `0` on failure and `1`
on success.

A querier can be deserialized (read from a file) in the following way:

```
  fout = fopen("mphf.out", "r");
  mphfq = MPHFDeserialize(fout);
  if(mphfq == NULL) {
    fprintf(stderr, "Deserialization failed...exiting\n");
    return -1;
  }
  fclose(fout);
```

Here, `fout` is of type `FILE *`. `mphfq` will be `NULL` on error.


# Linking

To use, simply link against `lib/libmphfsat.a` and include
`include/mphf.h`.


# Test

A sample interface is given in the `test` directory. The test builds
an MPHF for 30 random 10-byte elements and then queries the MPHF
against the original elements (for a consistency check) and prints
statistics. To run the test type:

```
$ make test/test && test/test
```


# Further Information

A paper about MPHF and SAT is available here: [Constructing Minimal
Perfect Hash Functions Using SAT
Technology](https://www.cs.cmu.edu/~mheule/publications/AAAI-WeaverS.1625.pdf).

