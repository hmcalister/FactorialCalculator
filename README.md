# Factorial Calculator

## Authors: Ryan Lau 3186050, Hayden McAlister 1663787

## Compile and run
* From the terminal run `gcc -o Choose Choose.c`
* For data entry from the terminal, run `./Choose` and enter n and k on the same line, space separated.
* For data entry from a file, run `./Choose -f < path` where `path` leads to a file containing n and k on the same line, space separated.
* For data validation from a file, run `./Choose -validate path` where path leads to a file with n, k, and the result of nCk all on one line space separated

## Testing
We have tested our program on the `test_data.txt` file using `./Choose -validate test_data.txt`. This file contains a bunch of combinations already performed from python, which we are assuming to be correct.

## Research
Using `C` and the `long long int` data type seems like a good idea as it is already 64 bit, but doesn't have extra precision. 

It seems like the issue is dealing with speed of the choose function and overflow of the data type, which is not checked inherently by `C`. It seems like the best solution may be to write our over choose function and check for overflow at each step, while ensuring speed. Hence, we choose `C` as our language as it is close to the metal and faster to run.

Let us investigate expected inputs and outputs to see if we can find a way to find errors.

Speaking mathematically:
```
Choose(n, k):
    definition:
        n! / k!(n-k)!
        Where ! is the factorial function

    Parameters
        n : integer, positive, greater than or equal to 0
            The size of the set we are choosing from

        k : integer, positive, greater than or equal to 0, less than or equal to n
            Number of objects to choose from set of size n

    Returns : integer, strictly greater than 0, often very large

    Notes:
        Choose(n,k) == Choose(n,n-k)

Factorial(n), n!:
    Definition:
        n * (n-1) * (n-2) * ... * 1

    Parameters
        n : integer, positive, greater than or equal to 0

    Returns : integer, at least as large as n, usually *much* larger
    
```

We currently think that a good solution will be to write our own implementation of the factorial and choose function which checks for overflow at each point. 64 bit integers can reach 9223372036854775807, which would easily blow the stack limit if we used recursion for factorial. So we should use an iterative factorial function instead, and at each step check for overflow. While we *could* extract the iterative factorial to a function with some parameters to avoid re-using code, it may be better to leave the code in one function to avoid excessive function calls (look into the compiler for this). We can also pass around pointers for even faster code.

### Checking for overflow
Any overflow would be an end to the calculations as we have broken the data type and cannot return anything meaningful. So detecting overflow early could save many CPU cycles. The best way to detect overflow would be to check the most significant bit. In C, `long long int` uses two's compliment, so we can ensure that any number with a most significant bit of 1 is negative. Since we are only expecting positive inputs and we are only using positive integers during calculations, we can safely say that *any* negative number *anywhere* in our program would be an overflow and can terminate the calculation.

Our test for overflow can therefore be checking the most significant bit. To ensure speed, we should try and do this as quickly as possible, probably working down at the bit level. The bitwise and operator, `&`, could prove useful as it can directly compare the sign bit with a value to determine sign. If we first cast our `long long int` to just `int` (after a bit shift `>>`) we can even return the result for a boolean expression.
```(int)(*n>>32) & 0x80000000```
This expression checks the most significant bit of the long n. With only one comparison we can quickly see if our long `n` is negative or not.

---

After debugging a program for a while, I have discovered that it is possible to overflow so far that we miss negatives entirely, and this check fails. Rewriting this check also allows us to support `unsigned long long int` and get even better performance, so it's not all bad. We now support up to `18,446,744,073,709,551,615`.

### On factorials
Factorials are expensive to compute, as they can take very long to iterate all the way down to 1 performing multiplication each time.

We can save time overall by keeping the results of each factorial we do, avoiding us having to compute the same iterations multiple times. For example, if we know that `n > k > n-k` we can first compute `(n-k)!` and save this result, and then only compute `k!` to the point of `(n-k)` at which time we can just use the previous result. Finally, we can do the same for `n!` and use the result of `k!` once we reach this point.

We can guarantee that `n > k > n-k` as `Choose(n,k) == Choose(n,n-k)`, so if `k < n-k` we can just swap `k` and `n-k`. Simple!

One thing to notice about factorials is that many terms appear in the top and bottom lines:
```
n! / k!(n-k)!
= n(n-1)(n-2)...(k+2)(k+1)(k)(k-1)(k-2)...(3)(2)(1) / (k)(k-1)(k-2)...(3)(2)(1) (n-k)(n-k-1)...(3)(2)(1)
= n(n-1)(n-2)...(k+2)(k+1)/(n-k)(n-k-1)...(3)(2)(1)
```
So that's a lovely bit of speed up by not having to calculate k!. Although, we will still have to do the operation at some point, but this makes it much more likely that we won't overflow the 64 bit integer, as 
```
n(n-1)(n-2)...(k+2)(k+1)(k)(k-1)(k-2)...(3)(2)(1) << n(n-1)(n-2)...(k+2)(k+1)
```

### After some testing
The 64-bit signed integer limit is of course `9,223,372,036,854,775,807`. However, this is quite lacking in the face of factorials, so squeezing out some extra digits would be nice. Because we are dealing with the choose function, we will never see a negative number, so moving over to 64-bit unsigned integers would be better, or `unsigned long long int`. This means we have to rework our check for overflow however, as the sign bit is no longer representative of a negative number. 

The new method is to check for overflow *before* multiplication, not after. Knowing that the `unsigned long long int` max integer is `18446744073709551615ULL` (call this `ULLI_MAX`), and knowing that we are only ever multiplying two numbers (let these be `a` and `b`) together at a time, we can say that overflow occurs when `a * b > ULLI_MAX`. But if we rearrange this, we can check if `a > ULLI_MAX / b` and now we can say for certain if the multiplication will result in overflow! If `a` is larger than `ULLI_MAX/b` then we cannot multiply `a` and `b` without getting a number over `ULLI_MAX`. Problem solved! We will perform this check before every multiplication in our factorial loops and return an error if overflow is detected.

Also, we used to use `-1` as the error code for our function, but after some *serious* debugging we finally realised that this will no longer work as `-1` doesn't exist in `unsigned long long int`. Luckily, the choose function is never zero, so we can safely return 0 as our error code. Phew!

### Even more efficent
Take the example 68C27. This overflows the current implementation but (we are assured) that this is calculatable!

The current implementation innocently calculates `(n-k)!` and `n!` separately. However, if either overflow then it is game over. If we instead recognise that they both have `k` terms and we can use this to our advantage! Now, we will perform a step of multiplication and division at the same time, keeping us from overflowing so often.

However, we can do even better! If we recognise an overflow is possible, we can try reducing our terms to get more perfomance. Let `r` be the current result, and we are calculating one term at a time of
```
r = n/(n-k) * (n-1)/(n-k-1) * (n-2)*(n-k-2) ...
```
So we are calculating step `i`:
```
r_+1 = r * (n-i)/(n-k-i)
```
Now let our numerator of this extra factor be `num = (n-i)` and our denominator `den = (n-k-i)`. We can check if there is a common factor between `num` and `den` (call this `f_1`) and there may be a common factor between `r` and `den` (call this `f_2`). We can write:
```
num = f_1 * reduced_num

r = f_2 * reduced_r

den = f_1 * f_2 * reduced_reduced_den
```
And of course, checking the calculation we can validate that this will still give us the same output. But now we can totally ignore the factors `f_1` and `f_2`, i.e. we can reduce the size of our numbers and possibly avoid overflow!
