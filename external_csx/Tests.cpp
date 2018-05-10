#include "stdafx.h"
#include "Funcs.h"
#define CATCH_CONFIG_MAIN
#include "Catch.hpp"

SCENARIO("MakePointer Test")
{
	GIVEN("void* base of 0, DWORD offset of 10") {
		void* base = 0;
		DWORD offset = 10;

		THEN("The result should be 10") {
			REQUIRE(MakePointer(base, offset) == (void*)10);
		}
	}

	GIVEN("DWORD* base of 0x1000, short offset of 10") {
		DWORD* base = (DWORD*)0x1000;
		short offset = 10;

		THEN("The return value should be 4106 (4096 + 10)") {
			REQUIRE(MakePointer(base, offset) == (void*)4106);
		}
	}

	GIVEN("HMODULE base of 10000, DWORD offset of -100") {
		HMODULE base = (HMODULE)1000;
		DWORD offset = -100;

		THEN("The return value should be 9900") {
			REQUIRE(MakePointer(base, offset) == (void*)900);
		}
	}
}

SCENARIO("CompareSignature Tests") {

	GIVEN("Compare Signature 'ABC', mask 'xxx', search data 'ABCD") {
		BYTE* pSignature = (BYTE*)"ABC";
		char* pMask = "xxx";
		BYTE* pSearchData = (BYTE*)"ABCD";
		THEN("CompareSignature should return TRUE (Input was a match)") {
			REQUIRE(CompareSignature(pSearchData, pSignature, pMask));
		}
	}

	GIVEN("Compare Signature 'ABD', mask 'xx?', search data 'AB0") {
		BYTE* pSignature = (BYTE*)"ABD";
		char* pMask = "xx?";
		BYTE* pSearchData = (BYTE*)"AB0";
		THEN("CompareSignature should return TRUE (AB0 == AB?)") {
			REQUIRE(CompareSignature(pSearchData, pSignature, pMask));
		}
	}

	GIVEN("Compare Signature '00101', mask 'xx?xx', search data '00001") {
		BYTE* pSignature = (BYTE*)"00101";
		char* pMask = "xx?xx";
		BYTE* pSearchData = (BYTE*)"00001";
		THEN("CompareSignature should return TRUE (00001 == 00?01)") {
			REQUIRE(CompareSignature(pSearchData, pSignature, pMask));
		}
	}

	GIVEN("Compare Signature '?????', mask 'xxx?x', search data '?????") {
		BYTE* pSignature = (BYTE*)"?????";
		char* pMask = "xxx?x";
		BYTE* pSearchData = (BYTE*)"?????";
		THEN("CompareSignature should return TRUE ('???(?)?', == ?????)") {
			REQUIRE(CompareSignature(pSearchData, pSignature, pMask));
		}
	}

	GIVEN("Compare Signature '?????', mask 'xxxxx', search data '??3???") {
		BYTE* pSignature = (BYTE*)"?????";
		char* pMask = "xxxxx";
		BYTE* pSearchData = (BYTE*)"??3???";
		THEN("CompareSignature should return FALSE ('?????', != ??3??)") {
			REQUIRE(!CompareSignature(pSearchData, pSignature, pMask));
		}
	}

	GIVEN("Compare Signature '1, 2, 3, four', mask 'xxxxxx?xxxxxx', search data '??3???") {
		BYTE* pSignature = (BYTE*)"?????";
		char* pMask = "xxxxx";
		BYTE* pSearchData = (BYTE*)"??3???";
		THEN("CompareSignature should return FALSE ('?????', != ??3??)") {
			REQUIRE(!CompareSignature(pSearchData, pSignature, pMask));
		}
	}

	GIVEN("Compare Signature 'small-ish', mask 'xxxxxxxxxx', search data 'small") {
		BYTE* pSignature = (BYTE*)"TooBig";
		char* pMask = "xxxxxxxxxx";
		BYTE* pSearchData = (BYTE*)"small";
		THEN("CompareSignature should return FALSE ('?????', != ??3??)") {
			REQUIRE(!CompareSignature(pSearchData, pSignature, pMask));
		}
	}
}

SCENARIO("SignatureScan Tests") {

	GIVEN("Signature 'ABC', mask 'xxx', search data 'ABC") {
		char* pSignature = "ABC";
		char* pMask = "xxx";
		BYTE* pSearchData = (BYTE*)"ABC";
		int dataSize = 3;
		THEN("SignatureScan should return pointer pSearchData + 0") {
			REQUIRE(SignatureScan(pSearchData, dataSize, pSignature, pMask) == pSearchData);
		}
	}

	GIVEN("Signature 'duck', mask '??xx', search data 'shoes and socks") {
		char* pSignature = "duck";
		char* pMask = "??xx";
		BYTE* pSearchData = (BYTE*)"shoes and socks";
		int dataSize = 15;
		THEN("SignatureScan should return pointer pSearchData + 10 (??ck == sock)") {
			REQUIRE(SignatureScan(pSearchData, dataSize, pSignature, pMask) == MakePointer(pSearchData, 10));
			
		}
	}

	GIVEN("Signature 'CAKE', mask 'xxxx', search data '00000001002004005") {
		char* pSignature = "CAKE";
		char* pMask = "xxxx";
		BYTE* pSearchData = (BYTE*)"00000001002004005";
		int dataSize = 17;
		THEN("SignatureScan should return pointer NULL") {
			REQUIRE(SignatureScan(pSearchData, dataSize, pSignature, pMask) == NULL);
		}
	}

	GIVEN("Signature '555', mask 'x??', search data '0005") {
		char* pSignature = "555";
		char* pMask = "x??";
		BYTE* pSearchData = (BYTE*)"0005";
		int dataSize = 4;
		THEN("SignatureScan should return pointer NULL") {
			REQUIRE(SignatureScan(pSearchData, dataSize, pSignature, pMask) == NULL);
		}
	}
	GIVEN("Signature 0xBA 0xDF 0x00 0xD0 (Four bytes), mask 'xx??', search data 'xxyyzz1300 0xBA 0xDF 0xAC 0xE0' (string & bytes)") {
		char* pSignature = "\xBA\xDF\x00\xD0";
		char* pMask = "xx??";
		BYTE* pSearchData = (BYTE*)"xxyyzz1300\xBA\xDF\xAC\xE0";
		int dataSize = 14;
		THEN("SignatureScan should return pointer pSearchData + 10 (\xBA\xDF\xAC\xE0 == \xBA\xDF??)") {
			REQUIRE(SignatureScan(pSearchData, dataSize, pSignature, pMask) == MakePointer(pSearchData, 10));

		}
	}

	GIVEN("Signature '\x33' (byte), mask 'x', search data '\\xAA\\xBB\\xCC\\xDD\\xEE\\xFF\\x00\\x11\\x22\\x33' (bytes)") {
		char* pSignature = "\x33"; 
		char* pMask = "x";
		BYTE* pSearchData = (BYTE*)"\xAA\xBB\xCC\xDD\xEE\xFF\x00\x11\x22\x33";
		int dataSize = 10;
		THEN("SignatureScan should return pointer pSearchData + 9 (\x33 == \x33)") {
			REQUIRE(SignatureScan(pSearchData, dataSize, pSignature, pMask) == MakePointer(pSearchData, 9));

		}
	}

	GIVEN("Signature '\\x33' (byte), mask 'x', search data '\\xAA\\xBB\\x33' (bytes)") {
		char* pSignature = "\x33";
		char* pMask = "x";
		BYTE* pSearchData = (BYTE*)"\xAA\xBB\x33";
		int dataSize = 3;
		THEN("SignatureScan should return pointer pSearchData + 2 (\\x33 == \\x33)") {
			REQUIRE(SignatureScan(pSearchData, dataSize, pSignature, pMask) == (void*)MakePointer(pSearchData, 2));
		}
	}
}