/**
 * Template Max, used to use to ensure we do not have zero sized arrays.
 * The usage pattern is `float name[ArrayMax<SIZE, 1>.value]` and the array will always be at least 1
 */
struct ArrayMax<let M : int, let N : int>
{
    static const int value = (M < N) ? N : M;
}
