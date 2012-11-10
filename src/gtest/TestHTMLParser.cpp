#include <gtest/gtest.h>
#include "HTMLparse.h"
#include "Options.h"


bool endsWithSlash(const std::string& url)
{
	return url[url.size()-1] == '/';
}

TEST(TestHTMLParser, testparseurls1) {
// Test we fully parse the NPR HTML page
	std::cout << "hola" << std::endl;
	Options options("/");
	HTMLparse parser("http://www.a.com/","",".",options,"index.html");
//	ASSERT_FALSE(parser.out_bad());
	ASSERT_FALSE(parser.in_bad());
	std::vector<URLcontainer> urls;
	try
	{
		while (1)
		{
			URLcontainer urlcon = parser.get_href(options);
			urls.push_back(urlcon);
			std::cout << "'" << urlcon.ret_URL() << "'" << std::endl;
		}
	}
	catch (HTMLparseExceptionType e)
	{
	}
	for (std::vector<URLcontainer>::const_iterator i = urls.begin(); i != urls.end(); ++i)
	{
		if (i->ret_URL().at(0) >= '0' && i->ret_URL().at(0) <= '9')
			EXPECT_PRED1(endsWithSlash, i->ret_URL());
	}
	ASSERT_EQ(urls.size(), 99);
}

int main(int argc, char **argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

