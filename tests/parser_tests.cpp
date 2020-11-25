#include <gtest/gtest.h>

#include <sstream>

#include "vypcomp/parser/parser.h"

using namespace ::testing;

using namespace vypcomp;

class ParserTests : public Test {};

TEST_F(ParserTests, emptyInput)
{
	std::stringstream input;
	std::ostringstream output;

	LangParser parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, invalidFile)
{
	LangParser parser;
	ASSERT_THROW(parser.parse("pls_dont_create_file_with_this_name"), std::runtime_error);
}

TEST_F(ParserTests, supportSimpleMain)
{
        std::stringstream input(R"(
                void main(void) {
                        return;
                }
        )");

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
	ASSERT_THROW(parser.parse(input), SyntaxError);
}

TEST_F(ParserTests, supportAssignmentOfLiterals)
{
        std::stringstream input(R"(
                void main(void) {
			int a,b; string s;
			a = 0;
			b = 1;
			s = "\"Hello\x000020World!\"\n";
			s = "\x00010Dty\x000159i";
                        return;
                }
        )");

        LangParser parser;
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

        LangParser parser;
	ASSERT_THROW(parser.parse(input), SemanticError);
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

        LangParser parser;
	ASSERT_NO_THROW(parser.parse(input));
}

/**
 * Bonus: INITVAR, FLOAT
 */
TEST_F(ParserTests, supportAssignmentInitialization2)
{
        std::stringstream input(R"(
                void main(void) {
			int a = 0, b = 1; string s = "Hello world!", float f = 0.0;
                        return;
                }
        )");

        LangParser parser;
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
			f= = .1;
                        return;
                }
        )");

        LangParser parser;
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
			foo_return0();

                        return;
                }
        )");

        LangParser parser;
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
			foo_return0();

                        return;
                }
        )");

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
	ASSERT_THROW(parser.parse(input), SemanticError);
}

TEST_F(ParserTests, supportDefinedAfterCall)
{
        std::stringstream input(R"(
                void main(void) {
			foo();
                        return;
                }
		int foo(void) {
			return 0;
		}
        )");

        LangParser parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, semanticErrorRedefinition)
{
        std::stringstream input(R"(
		int foo(void) {
			return 0;
		}
                void main(void) {
			foo();
                        return;
                }
		int foo(void) {
			return 0;
		}
        )");

        LangParser parser;
	ASSERT_THROW(parser.parse(input), SemanticError);
}

TEST_F(ParserTests, semanticErrorRedefinitionParameeter)
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

        LangParser parser;
	ASSERT_THROW(parser.parse(input), SemanticError);
}

TEST_F(ParserTests, semanticErrorRedefinitionLocalVar)
{
        std::stringstream input(R"(
		int foo(int bar) {
			int foo;
			return 0;
		}
                void main(void) {
			foo();
                        return;
                }
        )");

        LangParser parser;
	ASSERT_THROW(parser.parse(input), SemanticError);
}

TEST_F(ParserTests, semanticErrorRedefinitionLocalVar2)
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
	ASSERT_THROW(parser.parse(input), SemanticError);
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

        LangParser parser;
	ASSERT_THROW(parser.parse(input), SemanticError);
}

TEST_F(ParserTests, supportComments)
{
        std::stringstream input(R"(
		/**
		 * @brief This is main commented in block comment.
		 */
                int/*32_t*/ main(void/*, int*/) {
			// Main does not return anything.
                        return /*0*/;
                } // This is fine.

		/*int a = 3*/
        )");

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
	ASSERT_NO_THROW(parser.parse(input));
}

// Semicolon as terminator means that it ends statemets (not separates).
TEST_F(ParserTests, semicolonIsNotSeparator)
{
        std::stringstream input(R"(
                int Main(void) {
                        return 0;;
                }
        )");

        LangParser parser;
	ASSERT_THROW(parser.parse(input), SyntaxError);
}

TEST_F(ParserTests, semicolonIsMissing)
{
        std::stringstream input(R"(
                int Main(void) {
                        return 0
                }
        )");

        LangParser parser;
	ASSERT_THROW(parser.parse(input), SyntaxError);
}

TEST_F(ParserTests, semanticErrorMissingMain)
{
        std::stringstream input(R"(
                int Main(void) {
                        return 0;
                }
        )");

        LangParser parser;
	ASSERT_THROW(parser.parse(input), SemanticError);
}

TEST_F(ParserTests, semanticErrorInvalidMain)
{
        std::stringstream input(R"(
                int main(void) {
                        return 0;
                }
        )");

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
	ASSERT_THROW(parser.parse(input), SemanticError);
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
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

        LangParser parser;
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
				super.foo();
			}
		}
                void main(void) {
                }
        )");

        LangParser parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportThisAccess)
{
        std::stringstream input(R"(
		class Class : Object {
			void foo(int ok) {
				this.ok = ok;
			}
			int ok;
		}
                void main(void) {
                }
        )");

        LangParser parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportRegularAccess)
{
        std::stringstream input(R"(
		class Class : Object {
			void foo(int ok1) {
				ok = ok1;
			}
			int ok;
		}
                void main(void) {
                }
        )");

        LangParser parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportInstantiation)
{
        std::stringstream input(R"(
		class Class : Object {
			void foo(int ok) {
				this.ok = ok;
			}
			int ok;
		}
                void main(void) {
			Class a = new Class;
			a.foo(0);
                }
        )");

        LangParser parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportOverride)
{
        std::stringstream input(R"(
		class Class : Object {
			void foo(int a, int b) {
				this.c = a+b;
			}
			void foo(int a) {
				this.c = c*a;
			}
			int ok;
		}
                void main(void) {
			Class a = new Class;
			a.foo(0);
                }
        )");

        LangParser parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, semanticErrorRedefinitionMethod)
{
        std::stringstream input(R"(
		class Class : Object {
			void foo(int a) {
				this.c = a;
			}
			void foo(int a) {
				this.c = c*a;
			}
			int ok;
		}
                void main(void) {
			Class a = new Class;
			a.foo(0);
                }
        )");

        LangParser parser;
	ASSERT_THROW(parser.parse(input), SemanticError);
}

TEST_F(ParserTests, supportCallParentConstructor)
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
			void derived(void) {
				parent();
			}
		}
                void main(void) {
                }
        )");

        LangParser parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportSuperInstanceVarAccess)
{
        std::stringstream input(R"(
		class parent : Object {
			int ok;
		}

		class Class : parent {
			void foo(int ok) {
				super.ok = ok;
			}
		}
                void main(void) {
                }
        )");

        LangParser parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportSuperInstanceVarAccess2)
{
        std::stringstream input(R"(
		class parent : Object {
			int ok;
		}

		class Class : parent {
			void foo(int nok) {
				ok = nok;
			}
		}
                void main(void) {
                }
        )");

        LangParser parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, supportPolymorphism)
{
        std::stringstream input(R"(
		class parent : Object {
			int ok;
		}

		class Class : parent {
			void foo(int nok) {
				ok = nok;
			}
		}
                void main(void) {
			parent b = new Class;
                }
        )");

        LangParser parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, semanticErrorNonPolymorphism)
{
        std::stringstream input(R"(
		class parent : Object {
			int ok;
		}

		class Class : parent {
			void foo(int nok) {
				ok = nok;
			}
		}
                void main(void) {
			Class b = new parent;
                }
        )");

        LangParser parser;
	ASSERT_NO_THROW(parser.parse(input));
}

TEST_F(ParserTests, testSubsumptionAndIndirectness)
{
        std::stringstream input(R"(
		int bla(Class b) : Object {
			return b.foo(0);
		}

		class Class : Object {
			int foo(int nok) {
				return nok+42;
			}
		}

		class ClassB : Class {
		}

		class ClassC : ClassB {
		}

                void main(void) {
			Class a = new Class;
			ClassB b = new ClassB;
			ClassC c = new ClassC;
			bla(a); bla(b); bla(c);
                }
        )");

        LangParser parser;
	ASSERT_NO_THROW(parser.parse(input));
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

        LangParser parser;
	ASSERT_NO_THROW(parser.parse(input));
}
