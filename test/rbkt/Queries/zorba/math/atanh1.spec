# atanh(0.3) = 0.3095196042031117033...; glibc rounds correctly, Apple's libm
# returns the next double up. Shortest round-trip output shows the difference.
Result: $RBKT_SRC_DIR/ExpQueryResults/zorba/math/atanh1.xml.res
Result: $RBKT_SRC_DIR/ExpQueryResults/zorba/math/atanh1-apple-libm.xml.res
