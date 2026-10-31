# atanh of 0.3 is 0.3095196042031117033...; glibc rounds correctly, the libm
# of macOS and Ubuntu arm64 returns the next double up. Shortest round-trip
# output shows the difference.
# No parentheses in these comments: the spec tokenizer mishandles them.
Result: $RBKT_SRC_DIR/ExpQueryResults/zorba/math/atanh1.xml.res
Result: $RBKT_SRC_DIR/ExpQueryResults/zorba/math/atanh1-next-double.xml.res
