# mphf
Minimal Perfect Hash Functions using SAT

# A Description

Say we have a set of keywords `W = {w_0, ..., w_{n-1}}`. We want to
create a bijective function that maps `W` to the integers `{0, ...,
n-1}`.

Satisfiability can be used to do exactly this!

First, use `k = lg2 n` hash functions to hash each word `w_i` to `k`
variables in the range {0, ..., m-1}, where m > 1.44n.

```
<H_0(w_0), ..., H_{k-1}(w_0)>
...
<H_0(w_{n-1}), ..., H_{k-1}(w_{n-1})>
```

So, for example, if `n=8` and `m=12`, then `k=3` and we could get (depending on the hash functions used):

```
w_0 -> H -> <3, 9, 11>
w_1 -> H -> <0, 1, 9>
...
w_7 -> H -> <11, 2, 3>
```

Next, we want to find an assignment to each of the `m` Boolean
variables that causes each bitvector (vector of Booleans) to be
different.

For example, `<0=T, 1=F, 2=T, 3=T, 4=T, 5=F, 6=T, 7=T, 8=F, 9=T, 10=T, 11=F>`

This "solution" creates the following mapping

```
w_0 -> H -> <3, 9, 11> -> 0b110 -> 6. So, w_0 maps to 6.
w_1 -> H -> <0, 1, 9>  -> 0b101 -> 5. So, w_1 maps to 5.
...
w_7 -> H -> <11, 2, 3> -> 0b011 -> 3. So, w_7 maps to 3.
```

The following is not a valid solution, as `w_0` and `w_7` map to the
same integer, 2.

`<0=T, 1=F, 2=T, 3=F, 4=T, 5=F, 6=T, 7=T, 8=F, 9=T, 10=T, 11=F>`
```
w_0 -> H -> <3, 9, 11> -> 0b010 -> 2. So, w_0 maps to 2.
w_1 -> H -> <0, 1, 9>  -> 0b101 -> 5. So, w_1 maps to 5.
...
w_7 -> H -> <11, 2, 3> -> 0b010 -> 2. So, w_7 maps to 2.
```
