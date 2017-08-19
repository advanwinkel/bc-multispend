# bc-multispend
Building a multisig bitcoin transaction in C++ using libbitcoin

Compile with:

`g++ -std=c++11  -o spendMultiSig spendMultiSig.cpp HD_Wallet.cpp $(pkg-config --cflags libbitcoin --libs libbitcoin)`
