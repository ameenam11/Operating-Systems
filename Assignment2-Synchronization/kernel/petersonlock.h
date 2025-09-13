struct petersonlock{
    uint locked;
    uint turn;
    int flags[2];
};