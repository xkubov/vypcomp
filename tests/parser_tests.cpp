/**
 * VYPa compiler project.
 * Authors: Peter Kubov (xkubov06), Richard Micka (xmicka11)
 */

#include <gtest/gtest.h>

#include <sstream>

#include "vypcomp/parser/parser.h"

using namespace ::testing;

using namespace vypcomp;

class ParserTests : public Test {};

TEST_F(ParserTests, invalidFile)
{
	ParserDriver parser;
	ASSERT_THROW(parser.parse("pls_dont_create_file_with_this_name"), std::runtime_error);
}

TEST_F(ParserTests, supportSimpleMain)
{
        std::stringstream input(R"(
                void main(void) {
                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportLocalVariablesSimple)
{
        std::stringstream input(R"(
                void main(void) {
			int a; string s;
                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

/**
 * Bonus: FLOAT
 */
TEST_F(ParserTests, supportLocalVariablesFloat)
{
        std::stringstream input(R"(
                void main(void) {
			int a; float f;
                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportLocalVariablesComma)
{
        std::stringstream input(R"(
                void main(void) {
			int a,b,c,d; string s;
                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, syntaxErrorMissingEitherColon)
{
        std::stringstream input(R"(
                void main(void) {
			int a b;
                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), SyntaxError);
}

TEST_F(ParserTests, supportDeclarationWithAssignment)
{
        std::stringstream input(R"(
                void main(void) {
			int a = 0, b = 32;
			string q = "Nice!";
                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportAssignmentOfLiterals)
{
        std::stringstream input(R"(
                void main(void) {
			int a,b; string s;
			a = 0;
			b = 1;
			s = "\"Hellox000020World!\"\n";
			s = "00010Dtyx000159i";
                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, semanticErrorInAssignment)
{
        std::stringstream input(R"(
                void main(void) {
			int a;
			a = "error";
                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), IncompabilityError);
}

/**
 * Bonus: INITVAR
 */
TEST_F(ParserTests, supportAssignmentInitialization)
{
        std::stringstream input(R"(
                void main(void) {
			int a = 0, b = 1; string s = "Hello world!";
                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

/**
 * Bonus: INITVAR, FLOAT
 */
TEST_F(ParserTests, supportAssignmentInitialization2)
{
        std::stringstream input(R"(
                void main(void) {
			int a = 0, b = 1; string s = "Hello world!"; float f = 0.0;
                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

/**
 * Bonus: FLOAT
 */
TEST_F(ParserTests, supportFloat)
{
        std::stringstream input(R"(
                void main(void) {
			float f;
			f = 0.0;
			f = 1.f;
			f = .1;
                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportFunctions)
{
        std::stringstream input(R"(
		int foo(void) {
			return 0;
		}
		int foo_return_0(void) {
		}
		string bar(void) {
			return "";
		}
		string bar_return_empty(void) {
		}
		void ok(void) {
			return;
		}
		void ok_return(void) {
		}
                void main(void) {
			foo();
			foo_return_0();

                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportFunctionParameters)
{
        std::stringstream input(R"(
		int foo(int a, string b, int c) {
			b = ""; c = 0;
			return a;
		}
		void ok_return(void) {
		}
                void main(void) {
			int a;
			a = foo(0, "", 0);
			ok_return();

                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportIgnoreReturnAndParams)
{
        std::stringstream input(R"(
		int foo(int a, string b, int c) {
			return a;
		}
                void main(void) {
			foo(0, "", 0);

                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, semanticErrorParameters)
{
        std::stringstream input(R"(
		int foo(int a, string b, int c) {
			b = ""; c = 0;
			return a;
		}
                void main(void) {
			int a;
			a = foo("", 0, "");
			foo_return0();

                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), IncompabilityError);
}

TEST_F(ParserTests, semanticErrorRedefinitionParameter)
{
        std::stringstream input(R"(
		int foo(int foo) {
			return 0;
		}
                void main(void) {
			foo();
                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), SemanticError);
}

TEST_F(ParserTests, semanticErrorRedefinitionLocalVar)
{
        std::stringstream input(R"(
		int foo(int bar) {
			int bar;
			return 0;
		}
                void main(void) {
			foo();
                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), SemanticError);
}

TEST_F(ParserTests, supportCaseSensitiveFunctionNames)
{
        std::stringstream input(R"(
		int Main(void) {
			return 0;
		}
                void main(void) {
                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, semanticErrorFunctionReturn)
{
        std::stringstream input(R"(
		int Main(void) {
			return "error";
		}
                void main(void) {
                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), IncompabilityError);
}

TEST_F(ParserTests, semanticErrorFunctionAssignment)
{
        std::stringstream input(R"(
		int Main(void) {
			return 0;
		}
                void main(void) {
			string a;
			a = Main();
                        return;
                }
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), IncompabilityError);
}

TEST_F(ParserTests, supportComments)
{
        std::stringstream input(R"(
		/**
		 * @brief This is main commented in block comment.
		 */
                void/*32_t*/ main(void/*, int*/) {
			// Main does not return anything.
                        return /*0*/;
                } // This is fine.

		/*int a = 3*/
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportCommentsInStrings)
{
        std::stringstream input(R"(
		void main(void) {
			string a;
			a = "well /* */ // ok";
			return;
		}
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, dontSupportNestedComments)
{
        std::stringstream input(R"(
		/**
		 * @brief This is main commented in block comment.
		 * /*
		 * */
		 */
                int/*32_t*/ main(void/*, int*/) {
			// Main does not return anything.
                        return /*0*/;
                } // This is fine.

		/*int a = 3*/
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), SyntaxError);
}

// Semicolon as terminator means that it ends statemets (not separates).
TEST_F(ParserTests, semicolonIsNotSeparator)
{
        std::stringstream input(R"(
                int Main(void) {
                        return 0;;
                }
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), SyntaxError);
}

TEST_F(ParserTests, semicolonIsMissing)
{
        std::stringstream input(R"(
                int Main(void) {
                        return 0
                }
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), SyntaxError);
}

TEST_F(ParserTests, semanticErrorMissingMain)
{
        std::stringstream input(R"(
                int Main(void) {
                        return 0;
                }
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), SemanticError);
}

TEST_F(ParserTests, semanticErrorInvalidMain)
{
        std::stringstream input(R"(
                int main(void) {
                        return 0;
                }
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), SemanticError);
}

TEST_F(ParserTests, supportIfStatement)
{
        std::stringstream input(R"(
                void main(void) {
			int a;
			if (a) {
			} else {
			}
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportStatementsInIfStatement)
{
        std::stringstream input(R"(
                void main(void) {
			int a;
			if (a) {
				int b;
				int c;
			} else {
				int e,f;
			}
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

/**
 * Bonus: SCOPE
 */
TEST_F(ParserTests, supportScopeInIf)
{
        std::stringstream input(R"(
                void main(void) {
			int a;
			if (a) {
				int a;
			} else {
				int a;
			}
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportWhile)
{
        std::stringstream input(R"(
                void main(void) {
			int a;
			while (a) {
			}
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportStatmenetsInWhile)
{
        std::stringstream input(R"(
                void main(void) {
			int a;
			while (a) {
				int b;
				int c;
				int d;
			}
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, syntaxErrorExpectedExpression)
{
        std::stringstream input(R"(
                void main(void) {
			int a;
			if (a = 0) {
			} else {
			}
                }
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), SyntaxError);
}

TEST_F(ParserTests, syntaxErrorExpectedExpression2)
{
        std::stringstream input(R"(
                void main(void) {
			int a;
			if () {
			} else {
			}
                }
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), SyntaxError);
}

TEST_F(ParserTests, supportFunctionCallExpression)
{
        std::stringstream input(R"(
		int foo(void) {
		}
                void main(void) {
			if (foo()) {
			} else {
			}
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportExpressions)
{
        std::stringstream input(R"(
                void main(void) {
			int a, b, c;
			c = a+b;
			c = a-b;
			c = a<=b;
			c = !a;
			c = a*b/a;
			c = a>=b;
			c = a<b;
			c = a>b;
			c = a && b;
			c = a || b;
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, invalidOperands)
{
        std::stringstream input(R"(
                void main(void) {
			int a;
			string b;
			int c = a + b;
                }
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), IncompabilityError);
}

TEST_F(ParserTests, supportChaoticalExpressions)
{
        std::stringstream input(R"(
                void main(void) {
			int a, b;
			if (!(a*b) && (a/b) || ((a+(b <= a*b) && a)-b >= a/b) == a) {
			} else {
			}
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportEmbededFunctions)
{
        std::stringstream input(R"(
                void main(void) {
			int a;
			string b, c;
			print("Well!\n");
			a = readInt();
			b = readString();
			a = length(b);
			b = subStr(b, 0, 10);
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportClasses)
{
        std::stringstream input(R"(
		class test : Object {
		}
                void main(void) {
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportInheritance)
{
        std::stringstream input(R"(
		class parent : Object {
		}
		class derived : parent {
		}
                void main(void) {
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportMethods)
{
        std::stringstream input(R"(
		class parent : Object {
			void foo(void) {
				int ok;
			}
			void bar(void) {
				int ok;
			}
		}
		class derived : parent {
		}
                void main(void) {
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportInstanceVars)
{
        std::stringstream input(R"(
		class parent: Object  {
			int ok;
		}
		class derived : parent {
		}
                void main(void) {
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportSameNameMethods)
{
        std::stringstream input(R"(
		class parent : Object {
			void foo(void) {
				int ok;
			}
		}
		class derived : parent {
			void foo(void) {
				int ok;
			}
		}
                void main(void) {
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportSuperAccess)
{
        std::stringstream input(R"(
		class parent : Object {
			void foo(void) {
				int ok;
			}
		}
		class derived : parent {
			void foo(void) {
				super.foo();
			}
		}
                void main(void) {
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

/*BONUS*/
TEST_F(ParserTests, supportVisibility)
{
        std::stringstream input(R"(
		class Test : Object {
			public void setFoo(void) {
			}
			private int foo;
		}
                void main(void) {
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

/*BONUS*/
TEST_F(ParserTests, supportVisibilityViolation)
{
        std::stringstream input(R"(
		class TesetFoo: Object {
			private int foo;
			public void setFoo(void) {
				this.foo = 10;
			}
		}
                void main(void) {
			Test test;
			test.foo = 3;
                }
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), SemanticError);
}

TEST_F(ParserTests, supportCustomTypes)
{
        std::stringstream input(R"(
		class parent: Object  {
			int ok;
		}
		class derived : parent {
		}
		parent foo(void) {
		}
                void main(void) {
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportConstructor)
{
        std::stringstream input(R"(
		class parent : Object {
			void parent(void) {
				int ok;
			}
			void foo(void) {
				int ok;
			}
		}
		class derived : parent {
			void foo(void) {
			}
		}
                void main(void) {
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportConstructorError)
{
        std::stringstream input(R"(
		class parent : Object {
			int parent(void) {
				int ok;
			}
			void foo(void) {
				int ok;
			}
		}
		class derived : parent {
			void foo(void) {
			}
		}
                void main(void) {
                }
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), SemanticError);
}

TEST_F(ParserTests, supportConstructorError2)
{
        std::stringstream input(R"(
		class parent : Object {
			void parent(int blu) {
				int ok;
			}
			void foo(void) {
				int ok;
			}
		}
		class derived : parent {
			void foo(void) {
			}
		}
                void main(void) {
                }
        )");

        ParserDriver parser;
	ASSERT_THROW(parser.parse(input), SemanticError);
}

TEST_F(ParserTests, testEmbededMethods)
{
        std::stringstream input(R"(
		class Class : Object {
			int foo(int nok) {
				return nok+42;
			}
		}

                void main(void) {
			Class a = new Class;
			print(a.toString());
			print(a.getClass());
                }
        )");

        ParserDriver parser;
	ASSERT_NO_THROW(parser.parse(input));
}

//
// Expression parsing unit tests
//
TEST_F(ParserTests, testExpressionBinaryOp)
{
	std::stringstream input("12 + 34");

	ParserDriver parser;
	parser.parseExpression(input, 0);
	printf("finished");
}

TEST_F(ParserTests, testExpressionPrecedence)
{
	std::stringstream input("669 / 12 + 34 * 45");

	ParserDriver parser;
	parser.parseExpression(input, 0);
	printf("finished");
}

TEST_F(ParserTests, testExpressionPrecedence1)
{
	std::stringstream input("74 * 12 * 34 + 45");

	ParserDriver parser;
	parser.parseExpression(input, 0);
	printf("finished");
}

TEST_F(ParserTests, testExpressionPrecedenceParentheses)
{
	std::stringstream input("74 * ( 21 + 12 ) * 34 + 45");

	ParserDriver parser;
	parser.parseExpression(input, 0);
	printf("finished");
}

TEST_F(ParserTests, literalExpressions)
{
	std::string inputs[] = { {"1337"}, {"\"hello string literal\""}, {"3.14159"} };
	for (const auto& input_string : inputs)
	{
		ParserDriver parser;
		std::stringstream input(input_string);
		ASSERT_NO_THROW(parser.parseExpression(input, 0));
	}
}

TEST_F(ParserTests, binaryOpExpressions)
{
	std::string inputs[] = { 
		{"13 + 37"}, {"\"hello string literal\" + \"string to concat\""}, /*{"3.14159+1.23"} TODO: when floats are supported */
		{"58 - 78"},
		{"58 * 78"},
		{"58 / 78"},
		{"58 == 78"},
		{"58 != 78"},
		{"58 > 78"},
		{"58 >= 78"},
		{"58 < 78"},
		{"58 <= 78"},
		{"58 && 78"},
		{"58 || 78"},
	};
	for (const auto& input_string : inputs)
	{
		ParserDriver parser;
		std::stringstream input(input_string);
		ASSERT_NO_THROW(parser.parseExpression(input, 0));
	}
}

TEST_F(ParserTests, invalidBinaryOpExpressions)
{
	std::string inputs[] = { 
		{"13+\"hello\""}, {"\"hello string literal\" + 16"},
		{"\"hey\"-57"}, {"49-\"test\""}
	};
	for (const auto& input_string : inputs)
	{
		ParserDriver parser;
		std::stringstream input(input_string);
		ASSERT_THROW(parser.parseExpression(input, 0), IncompabilityError);
	}
}
