
TestHTMLParser : TestHTMLParser.cpp
	g++ -ggdb TestHTMLParser.cpp -I.. -I../.. -c
	g++ -ggdb ../Options.o ../HTMLparse.o ../URLcontainer.o ../utils.o ../Command.o ../Proxyserver.o TestHTMLParser.o -lgtest -o TestHTMLParser

clean:
	rm TestHTMLParser

test-run : TestHTMLParser
	./TestHTMLParser
