#include "llvm/ADT/APInt.h"
#include "llvm/IR/ConstantRange.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <iostream>

using namespace llvm;
using namespace std;

static LLVMContext C;
static IRBuilder<NoFolder> Builder(C);
static std::unique_ptr<Module> M = llvm::make_unique<Module>("calc", C);
static const bool phiNode = true;

struct Token{
 enum Kind {
  Hash,
  WhiteSpace,
  OpenParen,
  CloseParen,
  Int,
  Arith,
  Argument,
  Comparison,
  True, False,
  
  Plus,
  Minus,
  Mult,
  Division,
  Modulo,
  
  Eq,
  Lt,
  Lte,
  Gt,
  Gte,
  Neq,
 
  A0,A1,A2,A3, A4, A5,
  If,
  Error,
  Eof
 };
 Kind K;
 std::string val;
};

struct Lexer{
 unsigned LineNum;
 Lexer():LineNum(1){}

  bool notIncrease = false;
  char ch;
  unsigned column = 0;
 Token getNextToken(std::string &ErrStr) {
  while(1) {
   if(!notIncrease) {
    if(!cin.get(ch))
     break;
   }
   notIncrease = false;
   switch(ch) {
   case '\n':
    LineNum++;
    column = 0;
    continue;
   case ' ':
   case '\0':
   case '\t':
   case '\r':
    column++;
    continue;
   case '#':
    if(column != 0) {
     ErrStr = std::string("Expect # at the beginning of the line instead found at line:column - ") + std::to_string(LineNum) + ":" + std::to_string(column);
     return Token{Token::Error};
    }
    while(cin.get(ch) && ch != '\n') {}
    LineNum++;
    continue;
   case '(':
    column++;
    return Token{Token::OpenParen, "("};
   case ')':
    column++;
    return Token{Token::CloseParen, ")"};
   case 'a':
    column++;
    if(cin.get(ch)) {
     column++;
     if(ch == '0') {
      return Token{Token::A0, "0"};
     } else if(ch == '1') {
      return Token{Token::A1, "1"};
     } else if(ch == '2') {
      return Token{Token::A2, "2"};
     } else if(ch == '3') {
      return Token{Token::A3, "3"};
     } else if(ch == '4') {
      return Token{Token::A4, "4"};
     } else if(ch == '5') {
      return Token{Token::A5, "5"};
     } else {
      ErrStr = "Argument variable is not between a0-a5 at line num: " + std::to_string(LineNum);
      return Token{Token::Error, "Error"};
     }
    } else {
      ErrStr = "Argument variable is not between a0-a5" + std::to_string(LineNum);
      return Token{Token::Error, "Error"};
    }
   case 't':
      column++;
      if(cin.get(ch) && ch == 'r')
       if(cin.get(ch) && ch == 'u')
        if(cin.get(ch) && ch == 'e') {
         column += 3;
         return Token{Token::True, "true"};
        }
      ErrStr = std::string("Unexpected character ") + ch + std::string(" at line num: ") + std::to_string(LineNum);
      return Token{Token::Error, "Error"};
   case 'f':
      column++;
      if(cin.get(ch) && ch == 'a')
       if(cin.get(ch) && ch == 'l')
        if(cin.get(ch) && ch == 's')
         if(cin.get(ch) && ch == 'e') {
          column += 4;
          return Token{Token::False, "false"};
         }
      ErrStr = std::string("Unexpected character ") + ch + std::string(" at line num: ") + std::to_string(LineNum);
      return Token{Token::Error, "Error"};
   case '+':
      column++;
      return Token{Token::Plus, "+"};
   case '*':
      column++;
      return Token{Token::Mult, "*"};
   case '/':
      column++;
      return Token{Token::Division, "/"};
   case '<':
      column++;
      if(cin.get(ch)) {
       column++;
       if(ch == '=') {
        return Token{Token::Lte, "<="};
       } else if(ch == ' ') { 
        notIncrease = true;
        return Token{Token::Lt, "<"};
       }
      } else {
        ErrStr = std::string("Don't expect ") + ch + std::string(" after < at line num: ") + std::to_string(LineNum);
        return Token{Token::Error, "Error"};
       }
   case '>':
      column++;
      if(cin.get(ch)) {
       if(ch == '=') {
        column++;
        return Token{Token::Gte, ">="};
       }
       else if(ch == ' ') {
        notIncrease = true;
        return Token{Token::Gt, ">"};
       }
       } else {
        ErrStr = std::string("Don't expect ") + ch + std::string(" after > at line num: ") + std::to_string(LineNum);
        return Token{Token::Error, "Error"};
       }
   case '=': {
      column++;
      if(cin.get(ch) && ch == '=') {
       column++;
       return Token{Token::Eq, "=="};
      }
      else {
       ErrStr = std::string("Expect = after = at linenum: ") + std::to_string(LineNum);
       return Token{Token::Error, "Error"};
      }
   }
   case '!':
      column++;
      if(cin.get(ch) && ch == '=') {
       column++;
       return Token{Token::Neq, "!="};
      }
      else {
       ErrStr = std::string("Expect = after ! at linenum: ") + std::to_string(LineNum);
       return Token{Token::Error, "Error"};
      }
    case '-': {
      column++;
      if(cin.get(ch)) {
       if(ch == ' ') {
        notIncrease = true;
        return Token{Token::Minus, "-"};
       }
       else if(ch >= '0' && ch <= '9') {
        string val = "-";
        do {
         column++;
         val.push_back(ch);
        } while(cin.get(ch) && ch >= '0' && ch <= '9');
        notIncrease = true;
        return Token{Token::Int, val};
       } else {
        ErrStr = std::string("Unexpexted char ") + ch + std::string(" after - at line num: ") + std::to_string(LineNum);
       return Token{Token::Error, "Error"};
       }
      } else {
       ErrStr = std::string("Expexted a number after - at line num: ") + std::to_string(LineNum);
       return Token{Token::Error, "Error"};
      }
    }
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
     string val;
     do {
      column++;
      val.push_back(ch);
     } while(cin.get(ch) && ch >= '0' && ch <= '9');
     notIncrease = true;
     return Token{Token::Int, val};
    }
    case 'i': {
        column++;
     if(cin.get(ch) && ch == 'f') {
        column++;
      return Token{Token::If, "if"};
     } else {
      ErrStr = std::string("Expecting f after character i instead found ") + std::to_string(ch) + " at line nume: " + std::to_string(LineNum) ;
     }
    }
    default:
       ErrStr = std::string("Unexpected stuff: ") + ch + std::string(" at line num: ") + std::to_string(LineNum);   
       return Token{Token::Error, "Error"};
   }
  }
  return Token{Token::Eof, "Eof"};
 }
};

 struct Parser {
  Parser(std::string &ErrStr):L(), ErrStr(ErrStr) {}
 
  Value* parse() {
   if(!consumeToken(ErrStr)) {
    return 0;
   }
   Value* val = parseExpression();
   if(!consumeToken(ErrStr)) {
    return 0;
   }
   if(currToken.K != Token::Eof) {
    return 0;
   }
   return val;
  }
  
  bool consumeToken(std::string &ErrStr) {
   currToken = L.getNextToken(ErrStr);
   if(currToken.K == Token::Error) {
    return false;
   } else {
    return true;
   }
  }
  std::string& ErrStr;
  Token currToken;
  Lexer L;

  Value* parseBooleanExpr() {
   switch(currToken.K) {
    case Token::OpenParen: {
     if(!consumeToken(ErrStr)) {
      return 0;
     }
     switch(currToken.K) {
      case Token::Lt:
      case Token::Lte:
      case Token::Gt:
      case Token::Gte:
      case Token::Eq:
      case Token::Neq: {
       Token opToken = currToken;
       if(!consumeToken(ErrStr)) {
        return 0;
       }
       Value* a = parseExpression();
       if(!a) return 0;
       if(!consumeToken(ErrStr)) {
        return 0;
       }
       Value* b = parseExpression();
       if(!b) return 0;
       if(!consumeToken(ErrStr)) {
        return 0;
       }
       if(currToken.K != Token::CloseParen) {
        ErrStr = std::string("Expected close param at line num: ") + std::to_string(L.LineNum) + std::string("instead of: ") + currToken.val;
        return 0;
       }
       switch(opToken.K) {
        case Token::Lt:
         return Builder.CreateICmpSLT(a, b);
        case Token::Lte:
         return Builder.CreateICmpSLE(a, b);
        case Token::Gt:
         return Builder.CreateICmpSGT(a, b);
        case Token::Gte:
         return Builder.CreateICmpSGE(a, b);
        case Token::Eq:
         return Builder.CreateICmpEQ(a, b);
        case Token::Neq:
         return Builder.CreateICmpNE(a, b);
       }
      }
      }
      }
    case Token::True: {
     return llvm::ConstantInt::getSigned(llvm::IntegerType::get(C, 1), 1);
    }
    case Token::False: {
     return llvm::ConstantInt::getSigned(llvm::IntegerType::get(C, 1), 0);
    }
    default:
     ErrStr = std::string("Unknown token: ") + currToken.val + std::string(" at line num: ") + std::to_string(L.LineNum);
     return 0; 
    }   
   }

  Value* parseExpression() {
   switch(currToken.K) {
    case Token::OpenParen: {
     if(!consumeToken(ErrStr)) {
      return 0;
     }
     switch(currToken.K) {
      case Token::OpenParen:
       ErrStr = std::string("Successive open parans not allowed at line:column - ") + std::to_string(L.LineNum) + ":" + std::to_string(L.column);
       return 0;
      case Token::If: {
        if(!consumeToken(ErrStr)) {
         return 0;
        }
        Value* cond = parseBooleanExpr();
        if(!cond) return 0;
        if(!consumeToken(ErrStr)) {
         return 0;
        }
       if(!phiNode) {
        Value* a = parseExpression();
        if(!a) return 0;
        if(!consumeToken(ErrStr)) {
         return 0;
        }
        Value* b = parseExpression();
        if(!b) return 0;
        if(!consumeToken(ErrStr)) {
         return 0;
        }
        if(currToken.K != Token::CloseParen) {
         ErrStr = std::string("Expected close param at line num: ") + std::to_string(L.LineNum) + std::string("instead of: ") + currToken.val;
         return 0;
        }
        return Builder.CreateSelect(cond, a, b);
       } else {
        Function* f = Builder.GetInsertBlock()->getParent();
        BasicBlock* thenBB = BasicBlock::Create(C, "then", f);
        BasicBlock* elseBB = BasicBlock::Create(C, "else");
        BasicBlock* mergeBB = BasicBlock::Create(C, "merge");
        Builder.CreateCondBr(cond, thenBB, elseBB);
        Builder.SetInsertPoint(thenBB); 
        Value* a = parseExpression();
        if(!a) return 0;
        if(!consumeToken(ErrStr)) {
         return 0;
        }
        Builder.CreateBr(mergeBB);
        thenBB = Builder.GetInsertBlock();
        f->getBasicBlockList().push_back(elseBB);
        Builder.SetInsertPoint(elseBB);
        Value* b = parseExpression();
        if(!b) return 0;
        if(!consumeToken(ErrStr)) {
         return 0;
        }
        Builder.CreateBr(mergeBB);
        elseBB = Builder.GetInsertBlock();
        f->getBasicBlockList().push_back(mergeBB);
        Builder.SetInsertPoint(mergeBB);
        PHINode* phiN = Builder.CreatePHI(Type::getIntNTy(C, 64), 2, "iftmp");
        phiN->addIncoming(a, thenBB);
        phiN->addIncoming(b, elseBB);
        return phiN;
       }
      }
      case Token::Plus:
      case Token::Minus:
      case Token::Mult:
      case Token::Modulo:
      case Token::Division:
      { 
       Token opToken = currToken;
       if(!consumeToken(ErrStr)) {
        return 0;
       }
       Value* a = parseExpression();
       if(!a) return 0;
       if(!consumeToken(ErrStr)) {
        return 0;
       }
       Value* b = parseExpression();
       if(!b) return 0;
       if(!consumeToken(ErrStr)) {
        return 0;
       }
       if(currToken.K != Token::CloseParen) {
        ErrStr = std::string("Expected close param at line num: ") + std::to_string(L.LineNum) + std::string("instead of: ") + currToken.val;
        return 0;
       }
       switch(opToken.K) {
        case Token::Plus:
         return Builder.CreateAdd(a, b);
        case Token::Minus:
         return Builder.CreateSub(a, b);
        case Token::Mult:
         return Builder.CreateMul(a, b);
        case Token::Modulo:
         return Builder.CreateSRem(a, b); 
        case Token::Division:
         return Builder.CreateSDiv(a, b);
       }

      } 
      default:
       Value* v = parseExpression();
       if(!consumeToken(ErrStr)) {
        return 0;
       }
       if(currToken.K != Token::CloseParen) {
        ErrStr = std::string("Expected close param at line num: ") + std::to_string(L.LineNum) + std::string("instead of: ") + currToken.val;
        return 0;
       }
       return v;
     }
    case Token::A0:
    case Token::A1:
    case Token::A2:
    case Token::A3:
    case Token::A4:
    case Token::A5: {
     Function* f = M->getFunction("f");
     int index = std::stoi(currToken.val);
     llvm::Function::arg_iterator it=f->arg_begin(); 
     while(index-->0)
      it++;
     return &*it;
    }
    case Token::Int: {
     StringRef str = StringRef(currToken.val);
     int bitsNeeded = APInt::getBitsNeeded(str, 10);
     if(currToken.val != "-9223372036854775808") {
      if( currToken.val[0] == '-' && bitsNeeded > 64) {
       ErrStr = std::string("Integer is bigger than 64 bit number: ") + currToken.val + " at line num: " + std::to_string(L.LineNum);
       return 0;
      } else if (currToken.val[0] != '-' && bitsNeeded > 63) {
       ErrStr = std::string("Integer is bigger than 64 bit number: ") + currToken.val + " at line num: " + std::to_string(L.LineNum);
       return 0;
      }
     }
     return llvm::ConstantInt::get(C, APInt(64, str, 10));
    }
    default:
     ErrStr = std::string("Token: ") + currToken.val + std::string(" not expected at line num: ") + std::to_string(L.LineNum);
     return 0;
   
   }
   }
  }
}; 

static int compile() {
  M->setTargetTriple(llvm::sys::getProcessTriple());
  std::vector<Type *> SixInts(6, Type::getInt64Ty(C));
  FunctionType *FT = FunctionType::get(Type::getInt64Ty(C), SixInts, false);
  Function *F = Function::Create(FT, Function::ExternalLinkage, "f", &*M);
  BasicBlock *BB = BasicBlock::Create(C, "entry", F);
  Builder.SetInsertPoint(BB);

  std::string ErrStr;
  Parser p(ErrStr);
  Value* RetVal = p.parse();
  cout << ErrStr << endl; //Value *RetVal = ConstantInt::get(C, APInt(64, 0));
  if(RetVal == 0)
   exit(1); 
  Builder.CreateRet(RetVal);
  assert(!verifyModule(*M, &outs()));
  M->dump();
  return 0;
}

int main(void) { return compile(); }
