void outer(bool x) {
    void inner(bool y) {
        outer(false);
    }

    print x;
    
    if (x)
        inner(true);
}

outer(true);