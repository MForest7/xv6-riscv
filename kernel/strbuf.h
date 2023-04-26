struct strbuf {
    char *buf;
    uint64 capacity;
    uint64 begin;
    uint64 end;
    uint64 savepoint;
};
