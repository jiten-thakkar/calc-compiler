#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ADT/APInt.h"
#include "llvm/IR/ConstantRange.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "utils.h"
#include <cassert>
#include <iostream>

using namespace llvm;
using namespace std;

#define totalVariables 10

static LLVMContext C;
static IRBuilder<true, NoFolder> Builder(C);
static std::unique_ptr<Module> M = llvm::make_unique<Module>("calc", C);
static std::map<int, AllocaInst*> indexValue; 
static const bool phiNode = true;
static bool check=false;

static long ga, gb, gc, gd, ge, gf;

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
  M0,M1,M2,M3,M4,M5,M6,M7,M8,M9,
  If,
  Error,
  Set,
  Seq,
  While,
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
  unsigned offset = -1;
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
    offset++;
    continue;
   case ' ':
   case '\0':
   case '\t':
   case '\r':
    offset++;
    column++;
    continue;
   case '#':
    offset++;
    if(column != 0) {
     ErrStr = std::string("Expect # at the beginning of the line instead found at line:column - ") + std::to_string(LineNum) + ":" + std::to_string(column);
     return Token{Token::Error};
    }
    while(cin.get(ch) && ch != '\n') {offset++;}
    LineNum++;
    offset++;
    continue;
   case '(':
    column++;
    offset++;
    return Token{Token::OpenParen, "("};
   case ')':
    column++;
    offset++;
    return Token{Token::CloseParen, ")"};
   case 'a':
    offset++;
    column++;
    if(cin.get(ch)) {
     column++;
    offset++;
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
   case 'm':
    column++;
    offset++;
    if(cin.get(ch)) {
     column++;
    offset++;
     if(ch == '0') {
      return Token{Token::M0, "0"};
     } else if(ch == '1') {
      return Token{Token::M1, "1"};
     } else if(ch == '2') {
      return Token{Token::M2, "2"};
     } else if(ch == '3') {
      return Token{Token::M3, "3"};
     } else if(ch == '4') {
      return Token{Token::M4, "4"};
     } else if(ch == '5') {
      return Token{Token::M5, "5"};
     } else if(ch == '6') {
      return Token{Token::M6, "6"};
     } else if(ch == '7') {
      return Token{Token::M7, "7"};
     } else if(ch == '8') {
      return Token{Token::M8, "8"};
     } else if(ch == '9') {
      return Token{Token::M9, "9"};
     } else {
      ErrStr = "Global variable is not between m0-m9 at line num: " + std::to_string(LineNum);
      return Token{Token::Error, "Error"};
     }
    } else {
      ErrStr = "Argument variable is not between a0-a5" + std::to_string(LineNum);
      return Token{Token::Error, "Error"};
    }
   case 's':
      column++;
    offset++;
      if(cin.get(ch) && ch == 'e')
       if(cin.get(ch)) {
         column += 2;
         offset += 2;
         if(ch == 't') 
          return Token{Token::Set, "set"};
         else if(ch == 'q')
          return Token{Token::Seq, "seq"};
       }
      ErrStr = std::string("Unexpected character ") + ch + std::string(" at line num: ") + std::to_string(LineNum);
      return Token{Token::Error, "Error"};
   case 'w':
      column++;
      offset++;
      if(cin.get(ch) && ch == 'h')
       if(cin.get(ch) && ch == 'i')
        if(cin.get(ch) && ch == 'l') 
         if(cin.get(ch) && ch == 'e') {
         column += 4;
         offset += 4;
         return Token{Token::While, "while"};
        }
      ErrStr = std::string("Unexpected character ") + ch + std::string(" at line num: ") + std::to_string(LineNum);
      return Token{Token::Error, "Error"};
   case 't':
      column++;
      offset++;
      if(cin.get(ch) && ch == 'r')
       if(cin.get(ch) && ch == 'u')
        if(cin.get(ch) && ch == 'e') {
         column += 3;
         offset += 3;
         return Token{Token::True, "true"};
        }
      ErrStr = std::string("Unexpected character ") + ch + std::string(" at line num: ") + std::to_string(LineNum);
      return Token{Token::Error, "Error"};
   case 'f':
      column++;
      offset++;
      if(cin.get(ch) && ch == 'a')
       if(cin.get(ch) && ch == 'l')
        if(cin.get(ch) && ch == 's')
         if(cin.get(ch) && ch == 'e') {
          column += 4;
          offset += 4;
          return Token{Token::False, "false"};
         }
      ErrStr = std::string("Unexpected character ") + ch + std::string(" at line num: ") + std::to_string(LineNum);
      return Token{Token::Error, "Error"};
   case '+':
      offset++;
      column++;
      return Token{Token::Plus, "+"};
   case '%':
      offset++;
      column++;
      return Token{Token::Modulo, "%"};
   case '*':
      offset++;
      column++;
      return Token{Token::Mult, "*"};
   case '/':
      column++;
      offset++;
      return Token{Token::Division, "/"};
   case '<':
      column++;
      offset++;
      if(cin.get(ch)) {
       column++;
       if(ch == '=') {
        offset++;
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
      offset++;
      if(cin.get(ch)) {
       if(ch == '=') {
        column++;
        offset++;
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
      offset++;
      if(cin.get(ch) && ch == '=') {
       column++;
       offset++;
       return Token{Token::Eq, "=="};
      }
      else {
       ErrStr = std::string("Expect = after = at linenum: ") + std::to_string(LineNum);
       return Token{Token::Error, "Error"};
      }
   }
   case '!':
      column++;
      offset++;
      if(cin.get(ch) && ch == '=') {
       column++;
       offset++;
       return Token{Token::Neq, "!="};
      }
      else {
       ErrStr = std::string("Expect = after ! at linenum: ") + std::to_string(LineNum);
       return Token{Token::Error, "Error"};
      }
    case '-': {
      column++;
      offset++;
      if(cin.get(ch)) {
       if(ch == ' ') {
        notIncrease = true;
        return Token{Token::Minus, "-"};
       } else if(ch >= '0' && ch <= '9') {
        string val = "-";
        do {
         column++;
         offset++;
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
      offset++;
      val.push_back(ch);
     } while(cin.get(ch) && ch >= '0' && ch <= '9');
     notIncrease = true;
     return Token{Token::Int, val};
    }
    case 'i': {
        column++;
        offset++;
     if(cin.get(ch) && ch == 'f') {
        column++;
        offset++;
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
  Parser(std::string &ErrStr):L(), ErrStr(ErrStr) {
   initiateVariables();
  }
 
  void initiateVariables() {
   for(int i=0; i<totalVariables; i++) {
    indexValue[i] = Builder.CreateAlloca(Type::getIntNTy(C, 64), 0, "m"+std::to_string(i));
    Builder.CreateStore(llvm::ConstantInt::getSigned(llvm::IntegerType::get(C, 64), 0), indexValue[i]);
   } 
  }

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
        if(currToken.K != Token::CloseParen) {
         ErrStr = std::string("Expected close param at line num: ") + std::to_string(L.LineNum) + std::string("instead of: ") + currToken.val;
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
       unsigned curr_offset = L.offset;
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
       if(!check) {
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
       } else {
        Value* intrinsic_args[2] = {a, b};
        Function* f = Builder.GetInsertBlock()->getParent();
        Function* overflow_f = M->getFunction("overflow_fail");
        switch(opToken.K) {
         case Token::Plus: {
          Function* intrinsic_f = Intrinsic::getDeclaration(&*M, Intrinsic::sadd_with_overflow, ArrayRef<Type *>(Type::getInt64Ty(C)));
          Value* v = Builder.CreateCall(intrinsic_f, ArrayRef<Value*>(intrinsic_args, 2));
          Value* result = Builder.CreateExtractValue(v, ArrayRef<unsigned>(0));
          Value* overflow = Builder.CreateExtractValue(v, ArrayRef<unsigned>(1));
          Value* cond = Builder.CreateICmpEQ(overflow, ConstantInt::get(Type::getInt1Ty(C), 0));
          BasicBlock* thenBB = BasicBlock::Create(C, "then", f);
          BasicBlock* elseBB = BasicBlock::Create(C, "else");
          BasicBlock* mergeBB = BasicBlock::Create(C, "merge");
          Builder.CreateCondBr(cond, thenBB, elseBB);
          Builder.SetInsertPoint(thenBB);
          Value* add_val = Builder.CreateAdd(a, b);
          Builder.CreateBr(mergeBB);
          thenBB = Builder.GetInsertBlock();
          Builder.SetInsertPoint(elseBB);
          f->getBasicBlockList().push_back(elseBB);
          Value* overflow_args[] = {ConstantInt::get(Type::getInt64Ty(C), curr_offset)};
          Value* overflow_val = Builder.CreateCall(overflow_f, ArrayRef<Value*>(overflow_args, 1));
          Builder.CreateBr(mergeBB);
          elseBB = Builder.GetInsertBlock();
          Builder.SetInsertPoint(mergeBB);
          f->getBasicBlockList().push_back(mergeBB);
          PHINode* phiN = Builder.CreatePHI(Type::getIntNTy(C, 64), 2);
          phiN->addIncoming(add_val, thenBB);
          phiN->addIncoming(ConstantInt::get(Type::getInt64Ty(C), 0), elseBB);
          return phiN;
          }
         case Token::Minus: {
          Function* intrinsic_f = Intrinsic::getDeclaration(&*M, Intrinsic::ssub_with_overflow, ArrayRef<Type *>(Type::getInt64Ty(C)));
          Value* v = Builder.CreateCall(intrinsic_f, ArrayRef<Value*>(intrinsic_args, 2));
          Value* result = Builder.CreateExtractValue(v, ArrayRef<unsigned>(0));
          Value* overflow = Builder.CreateExtractValue(v, ArrayRef<unsigned>(1));
          Value* cond = Builder.CreateICmpEQ(overflow, ConstantInt::get(Type::getInt1Ty(C), 0));
          BasicBlock* thenBB = BasicBlock::Create(C, "then", f);
          BasicBlock* elseBB = BasicBlock::Create(C, "else");
          BasicBlock* mergeBB = BasicBlock::Create(C, "merge");
          Builder.CreateCondBr(cond, thenBB, elseBB);
          Builder.SetInsertPoint(thenBB);
          Value* add_val = Builder.CreateSub(a, b);
          Builder.CreateBr(mergeBB);
          thenBB = Builder.GetInsertBlock();
          Builder.SetInsertPoint(elseBB);
          f->getBasicBlockList().push_back(elseBB);
          Value* overflow_args[] = {ConstantInt::get(Type::getInt64Ty(C), curr_offset)};
          Value* overflow_val = Builder.CreateCall(overflow_f, ArrayRef<Value*>(overflow_args, 1));
          Builder.CreateBr(mergeBB);
          elseBB = Builder.GetInsertBlock();
          Builder.SetInsertPoint(mergeBB);
          f->getBasicBlockList().push_back(mergeBB);
          PHINode* phiN = Builder.CreatePHI(Type::getIntNTy(C, 64), 2);
          phiN->addIncoming(add_val, thenBB);
          phiN->addIncoming(ConstantInt::get(Type::getInt64Ty(C), 0), elseBB);
          return phiN;
          }
         case Token::Mult: {
          Function* intrinsic_f = Intrinsic::getDeclaration(&*M, Intrinsic::smul_with_overflow, ArrayRef<Type *>(Type::getInt64Ty(C)));
          Value* v = Builder.CreateCall(intrinsic_f, ArrayRef<Value*>(intrinsic_args, 2));
          Value* result = Builder.CreateExtractValue(v, ArrayRef<unsigned>(0));
          Value* overflow = Builder.CreateExtractValue(v, ArrayRef<unsigned>(1));
          Value* cond = Builder.CreateICmpEQ(overflow, ConstantInt::get(Type::getInt1Ty(C), 0));
          BasicBlock* thenBB = BasicBlock::Create(C, "then", f);
          BasicBlock* elseBB = BasicBlock::Create(C, "else");
          BasicBlock* mergeBB = BasicBlock::Create(C, "merge");
          Builder.CreateCondBr(cond, thenBB, elseBB);
          Builder.SetInsertPoint(thenBB);
          Value* add_val = Builder.CreateMul(a, b);
          Builder.CreateBr(mergeBB);
          thenBB = Builder.GetInsertBlock();
          Builder.SetInsertPoint(elseBB);
          f->getBasicBlockList().push_back(elseBB);
          Value* overflow_args[] = {ConstantInt::get(Type::getInt64Ty(C), curr_offset)};
          Value* overflow_val = Builder.CreateCall(overflow_f, ArrayRef<Value*>(overflow_args, 1));
          Builder.CreateBr(mergeBB);
          elseBB = Builder.GetInsertBlock();
          Builder.SetInsertPoint(mergeBB);
          f->getBasicBlockList().push_back(mergeBB);
          PHINode* phiN = Builder.CreatePHI(Type::getIntNTy(C, 64), 2);
          phiN->addIncoming(add_val, thenBB);
          phiN->addIncoming(ConstantInt::get(Type::getInt64Ty(C), 0), elseBB);
          return phiN;
          }
         case Token::Modulo: {
          Value* cond = Builder.CreateICmpNE(b, ConstantInt::get(Type::getInt1Ty(C), 0));
          BasicBlock* thenBB = BasicBlock::Create(C, "then", f);
          BasicBlock* elseBB = BasicBlock::Create(C, "else");
          BasicBlock* mergeBB = BasicBlock::Create(C, "merge");
          Builder.CreateCondBr(cond, thenBB, elseBB);
          Builder.SetInsertPoint(thenBB);
          Value* add_val = Builder.CreateSRem(a, b);
          Builder.CreateBr(mergeBB);
          thenBB = Builder.GetInsertBlock();
          Builder.SetInsertPoint(elseBB);
          f->getBasicBlockList().push_back(elseBB);
          Value* overflow_args[] = {ConstantInt::get(Type::getInt64Ty(C), curr_offset)};
          Value* overflow_val = Builder.CreateCall(overflow_f, ArrayRef<Value*>(overflow_args, 1));
          Builder.CreateBr(mergeBB);
          elseBB = Builder.GetInsertBlock();
          Builder.SetInsertPoint(mergeBB);
          f->getBasicBlockList().push_back(mergeBB);
          PHINode* phiN = Builder.CreatePHI(Type::getIntNTy(C, 64), 2);
          phiN->addIncoming(add_val, thenBB);
          phiN->addIncoming(ConstantInt::get(Type::getInt64Ty(C), 0), elseBB);
          return phiN;
          }
         case Token::Division: {
          Value* cond1 = Builder.CreateICmpEQ(b, ConstantInt::get(Type::getInt64Ty(C), 0));
          Value* cond2 = Builder.CreateICmpEQ(b, ConstantInt::get(Type::getInt64Ty(C), -1));
          StringRef str = StringRef("-9223372036854775808");
          Value* cond3 = Builder.CreateICmpEQ(a, ConstantInt::get(Type::getInt64Ty(C), str, 10));
          Value* andCond = Builder.CreateAnd(cond2, cond3);
          Value* orCond = Builder.CreateOr(andCond, cond1);
          BasicBlock* thenBB = BasicBlock::Create(C, "then", f);
          BasicBlock* elseBB = BasicBlock::Create(C, "else");
          BasicBlock* mergeBB = BasicBlock::Create(C, "merge");
          Builder.CreateCondBr(orCond, thenBB, elseBB);
          Builder.SetInsertPoint(thenBB);
          Value* add_val = Builder.CreateSDiv(a, b);
          Builder.CreateBr(mergeBB);
          thenBB = Builder.GetInsertBlock();
          Builder.SetInsertPoint(elseBB);
          f->getBasicBlockList().push_back(elseBB);
          Value* overflow_args[] = {ConstantInt::get(Type::getInt64Ty(C), curr_offset)};
          Value* overflow_val = Builder.CreateCall(overflow_f, ArrayRef<Value*>(overflow_args, 1));
          Builder.CreateBr(mergeBB);
          elseBB = Builder.GetInsertBlock();
          Builder.SetInsertPoint(mergeBB);
          f->getBasicBlockList().push_back(mergeBB);
          PHINode* phiN = Builder.CreatePHI(Type::getIntNTy(C, 64), 2);
          phiN->addIncoming(add_val, thenBB);
          phiN->addIncoming(ConstantInt::get(Type::getInt64Ty(C), 0), elseBB);
          return phiN;
         }
        }
         
       }
      } 
      case Token::Set: {
       if(!consumeToken(ErrStr)) {
        return 0;
       }
       Value* e = parseExpression();
       if(!e) return 0;
       if(!consumeToken(ErrStr)) {
        return 0;
       }
       if(currToken.K != Token::M0 && currToken.K != Token::M1 && currToken.K != Token::M2 && currToken.K != Token::M3 && currToken.K != Token::M4
          && currToken.K != Token::M5 && currToken.K != Token::M6 && currToken.K != Token::M7 && currToken.K != Token::M8 && currToken.K != Token::M9) {
        ErrStr = std::string("Expecting one of the variables m0-m9 instead got ") + currToken.val + std::string(" at line:col - ") + 
                             std::to_string(L.LineNum) + ":" + std::to_string(L.column);
        return 0;
       }
       Token opToken = currToken;
       if(!consumeToken(ErrStr)) {
        return 0;
       }
       if(currToken.K != Token::CloseParen) {
        ErrStr = std::string("Expected close param at line num: ") + std::to_string(L.LineNum) + std::string("instead of: ") + currToken.val;
        return 0;
       }
       Builder.CreateStore(e, indexValue[std::stoi(opToken.val)]);
       return e;
      }
      case Token::Seq: {
       if(!consumeToken(ErrStr)) {
        return 0;
       }
       Value* e1 = parseExpression();
       if(!e1) return 0;
       if(!consumeToken(ErrStr)) {
        return 0;
       }
       Value* e2 = parseExpression();
       if(!e2) return 0;
       if(!consumeToken(ErrStr)) {
        return 0;
       }
       if(currToken.K != Token::CloseParen) {
        ErrStr = std::string("Expected close param at line num: ") + std::to_string(L.LineNum) + std::string("instead of: ") + currToken.val;
        return 0;
       }
       return e2;
      }
      case Token::While: {
       if(!consumeToken(ErrStr)) {
        return 0;
       }
       BasicBlock* entryBB = Builder.GetInsertBlock();
       Function* f = Builder.GetInsertBlock()->getParent();
       BasicBlock* condBB = BasicBlock::Create(C, "cond", f);
       BasicBlock* afterLoopBB = BasicBlock::Create(C, "afterLoop");
       BasicBlock* loopBB = BasicBlock::Create(C, "loop");
       Builder.CreateBr(condBB);
       Builder.SetInsertPoint(condBB);
       PHINode* phiN = Builder.CreatePHI(Type::getIntNTy(C, 64), 2, "condtmp");
       phiN->addIncoming(llvm::ConstantInt::get(Type::getIntNTy(C, 64), 0), entryBB);
       Value* cond = parseBooleanExpr();
       if(!cond) return 0;
       Builder.CreateCondBr(cond, loopBB, afterLoopBB);
       f->getBasicBlockList().push_back(loopBB);
       Builder.SetInsertPoint(loopBB);
       if(!consumeToken(ErrStr)) {
        return 0;
       }
       Value* e = parseExpression();
       if(!e) return 0;
       loopBB = Builder.GetInsertBlock();
       Builder.CreateBr(condBB);
       phiN->addIncoming(e, loopBB);
       Builder.SetInsertPoint(afterLoopBB);
       f->getBasicBlockList().push_back(afterLoopBB); 
       if(!consumeToken(ErrStr)) {
        return 0;
       }
       if(currToken.K != Token::CloseParen) {
        ErrStr = std::string("Expected close param at line num: ") + std::to_string(L.LineNum) + std::string("instead of: ") + currToken.val;
        return 0;
       }
       return phiN;
      }
      default:
       ErrStr = std::string("Expected non constant expression at line num: ") + std::to_string(L.LineNum) + std::string("instead of: ") + currToken.val;
       return 0;
     }
    case Token::A0: {
      return ConstantInt::get(Type::getInt64Ty(C), ga);
    }
    case Token::A1: {
      return ConstantInt::get(Type::getInt64Ty(C), gb);
    }
    case Token::A2: 
      return ConstantInt::get(Type::getInt64Ty(C), gc);
    case Token::A3:
      return ConstantInt::get(Type::getInt64Ty(C), gd);
    case Token::A4:
      return ConstantInt::get(Type::getInt64Ty(C), ge);
    /*case Token::A5: {
     Function* f = M->getFunction("f");
     int index = std::stoi(currToken.val);
     llvm::Function::arg_iterator it=f->arg_begin(); 
     while(index-->0)
      it++;
     return &*it;
    }*/
    case Token::A5:
      return ConstantInt::get(Type::getInt64Ty(C), gf);
    case Token::M0:
    case Token::M1:
    case Token::M2:
    case Token::M3:
    case Token::M4:
    case Token::M5:
    case Token::M6:
    case Token::M7:
    case Token::M8:
    case Token::M9: {
     return Builder.CreateLoad(indexValue[std::stoi(currToken.val)]); 
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

long f(long a, long b, long c, long d, long e, long f) {
  ga = a;
  gb = b;
  gc = c;
  gd = d;
  ge = e;
  gf = f;
  timer::Timer t1;
  M->setTargetTriple(llvm::sys::getProcessTriple());
  //std::vector<Type *> SixInts(6, Type::getInt64Ty(C));
  std::vector<Type *> SixInts;
  FunctionType *FT = FunctionType::get(Type::getInt64Ty(C), SixInts, false);
  Function *F = Function::Create(FT, Function::ExternalLinkage, "f", &*M);
  std::vector<Type*> pos(1, Type::getInt64Ty(C));
  FunctionType *overflow_ft = FunctionType::get(Type::getVoidTy(C), pos, false);
  Function* overflow_f = Function::Create(overflow_ft, Function::ExternalLinkage, "overflow_fail", &*M);
  BasicBlock *BB = BasicBlock::Create(C, "entry", F);
  Builder.SetInsertPoint(BB);

  std::string ErrStr;
  Parser p(ErrStr);
  Value* RetVal = p.parse();
  cout << ErrStr << endl;
  if(RetVal == 0)
   exit(1); 
  Builder.CreateRet(RetVal);
  assert(!verifyModule(*M, &outs()));
  cout << "ir time: " << t1.elapsed() << std::endl;
  M->dump();
    
  timer::Timer t2;
  //Create the JIT
  ExecutionEngine* EE = EngineBuilder(std::move(M)).create();
  cout << "jit time: " << t2.elapsed() << std::endl;
  std::vector<GenericValue> args;
  timer::Timer t3;
  GenericValue gv = EE->runFunction(F, args);
  cout << "execution time: " << t3.elapsed() << std::endl;
  timer::Timer t4;
  gv = EE->runFunction(F, args);
  cout << "execution time: " << t4.elapsed() << std::endl;
  cout << "value: " << gv.IntVal.getSExtValue() << std::endl; 
  return gv.IntVal.getSExtValue();
}

static int compile() {
  /*Timer t4;
  gv = EE->runFunction(F, args);
  cout << "execution time: " << t4.elapsed() << std::endl;
  cout << "value: " << gv.IntVal.getSExtValue() << std::endl;*/
  f(1000, 0, 0, 0, 0 , 0);
  return 0;
}

int main(int argc, char* argv[]) { 
 assert(argc <= 2);
 if(argc == 2) {
  assert(!strcmp(argv[1], "-check"));
  check = true;
 }
 return compile();
}