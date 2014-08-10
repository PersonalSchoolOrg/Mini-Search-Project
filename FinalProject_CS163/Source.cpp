#define _CRT_SECURE_NO_WARNINGS

#define B_0000  0
#define B_0001  1
#define B_0010  2
#define B_0011  3
#define B_0100  4
#define B_0101  5
#define B_0110  6
#define B_0111  7
#define B_1000  8
#define B_1001  9
#define B_1010  a
#define B_1011  b
#define B_1100  c
#define B_1101  d
#define B_1110  e
#define B_1111  f

#define _B2H(bits)  B_##bits
#define B2H(bits)   _B2H(bits)
#define _hEX(n)     0x##n
#define HEX(n)      _hEX(n)
#define _CCAT(a,b)  a##b
#define CCAT(a,b)   _CCAT(a,b)

#define BYTE(a,b)       HEX( CCAT(B2H(a),B2H(b)) )
#define WORD(a,b,c,d)   HEX( CCAT(CCAT(B2H(a),B2H(b)),CCAT(B2H(c),B2H(d))) )
#define _DWORD(a,b,c,d,e,f,g,h)  HEX( CCAT(CCAT(CCAT(B2H(a),B2H(b)),CCAT(B2H(c),B2H(d))),CCAT(CCAT(B2H(e),B2H(f)),CCAT(B2H(g),B2H(h)))) )

#define FILE_SEPERATE_NAME "DataFile"
#define FILENAME "d"
#define SEARCH_FILE_NAME "query.txt"
#define SEARCH_RESULT_FILE_NAME "1351035_1351050_Result.txt"
#define BOOST "Dictionary.txt"

#define MODE 1
#define MAXPOOL 500000
#define _MAXCHAR 37
#include<string>
#include<fstream>
#include <time.h>
#include <list>
#include <stack>
#include <Windows.h>
using namespace std;
static int totalSearchTime;
static double totalSize;
static unsigned int  curFile = 0;
static unsigned char simpleHash[257];
static int numQuery;
static int getNumFile(){
	int numfile=0;
	FILE* pFile=fopen("d0000.txt", "r");
	fscanf(pFile, "%d", &numfile);
	fscanf(pFile, "%d", &numQuery);
	return numfile;
}
static const int numFile = getNumFile();
/*
static unsigned long long * initialize(){
	unsigned long long* abit = new unsigned long long[numFile];
	for (int i = 0; i < numFile ; ++i) abit[i] = (1i64 << i);
	return abit;
}
static unsigned long long* abit = initialize();
*/
static unsigned int fileLen[1000];
static char FileName[1000][100];
static unsigned int maxSize(){
	string fileName;
	char anyBuffer[1000];
	FILE * pFile;
	register unsigned int fLen, result = 0;
	for (curFile = 0; curFile < numFile; ++curFile){
		fileName = FILENAME;
		_itoa(curFile+1, anyBuffer, 10);
		for (int i = 4; i > strlen(anyBuffer); --i) fileName += '0';
		fileName = fileName + anyBuffer + ".txt";
		strcpy(FileName[curFile],fileName.c_str());
		pFile = fopen(fileName.c_str(), "rb");
		if (pFile == NULL) break;
		fseek(pFile, 0, SEEK_END);
		fLen = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);
		totalSize += fLen;
		fileLen[curFile] = fLen;
		if (result < fLen) result = fLen;
		fclose(pFile);
	}
	return result;
}

static unsigned int countWord;
const int MaxSize = maxSize();

struct Node {
	Node* mChildren[_MAXCHAR];
	unsigned short* nFile;
	~Node(){
		delete [] nFile;
	}
};

static Node** InitData(){
	//Simple Hash
	for (int i = 0; i < 257; ++i) simpleHash[i] = _MAXCHAR + 1;
	for (int i = 48; i <= 57; ++i) simpleHash[i] = i - 21;
	for (int i = 97; i <= 122; ++i) simpleHash[i] = (i | BYTE(0010, 0000)) & (-97);
	for (int i = 65; i <= 90; ++i) simpleHash[i] = (i | BYTE(0010, 0000)) & (-97);
	simpleHash['\r'] = _MAXCHAR;
	//Data Pool Initialize
	Node** a;
	a = new Node*[MAXPOOL];
	for (int i = 0; i < MAXPOOL; ++i) {
		a[i] = new Node;
		for (int j = 1; j < _MAXCHAR; ++j) a[i]->mChildren[j] = 0;
		a[i]->nFile = new unsigned short[numFile];
		memset(a[i]->nFile, 0, numFile);
	}
	return a;
}

static Node** poolData=InitData();
static Node** currentData=poolData;
static unsigned int currentBlock;

struct Trie {
	Node* root;
	Trie() : root(new Node) {
		for (int j=1;j<_MAXCHAR;++j) root->mChildren[j]=0;
		root->nFile = new unsigned short[numFile];
		memset (root->nFile,0,numFile);
	};
	
	void destroyTrie(int DELMODE=0)
	{
		for (int i=0;i<MAXPOOL;++i) {
			delete poolData[i];
			if (!DELMODE) {
				poolData[i]=new Node;
				for (int j=1;j<_MAXCHAR;++j) poolData[i]->mChildren[j]=0;
				poolData[i]->nFile = new unsigned short[numFile];
				memset (poolData[i]->nFile,0,numFile);
			}
		}
		currentBlock = 0;
		currentData = poolData;
		if (DELMODE) {
			delete[] poolData;
			delete root;
		}
	}
	~Trie(){ destroyTrie(1); }
	inline void Destroy(){ destroyTrie(); }
	inline Node* search(char* Sstring,int len);
	static inline Node* searchFromNode(Node* root, char* Sstring, int len);
};

Node* Trie::search(char* Sstring,int len){
	Node* current=root,*newNode=root;
	bool err = 0, noNode = 0;
	for (int i=0;i<len;++i)
			if (!err){
				current = newNode;
				if (current->mChildren[Sstring[i]] != 0)  newNode = current->mChildren[Sstring[i]];
				else err = true;
			} else break;
	if (err) return NULL;
	else return newNode;

}

Node* Trie::searchFromNode(Node* root, char* Sstring, int len){
	Node* current = root, *newNode = root;
	bool err = 0, noNode = 0;
	for (int i = 0; i<len; ++i)
	if (!err){
		current = newNode;
		if (current->mChildren[Sstring[i]] != 0)  newNode = current->mChildren[Sstring[i]];
		else err = true;
	}
	else break;
	if (err) return NULL;
	else return newNode;
}

unsigned char * Buff;

void inputTxT_Build(Trie * trie)
{
	FILE * pFile = NULL;
	register unsigned int fLen, result = 0, j = 0, tmp;
	Node* current = trie->root, *Root = trie->root,*newNode=NULL;
	for (curFile = 0; curFile < numFile; ++curFile){
		fLen = fileLen[curFile];
		pFile = fopen(FileName[curFile], "rb");
		if (pFile == NULL) break;
		fread(Buff, fLen, 1, pFile);
		Buff[fLen] = ' ';
		Buff[fLen + 1] = 0;
		register unsigned int i , t;
		unsigned char * ip = Buff -1;
		do{
			i = *(++ip);
			tmp = simpleHash[i];
			if ( tmp <_MAXCHAR){
				Node* & newNode = current->mChildren[tmp];
				if (newNode==0){
					//newNode = poolData[currentBlock++];
					//currentBlock++;
					//newNode = *currentData;
					//++currentData;
					__asm{
						mov ecx, currentData;
						mov eax, [ecx];
					 	mov ebx, newNode;
						mov [ebx], eax;
						add currentData, 4;
					};
				}
				current = newNode;
			}
			else if (current!=Root) {
				++current->nFile[curFile];
				current = Root;
			}
		} while (i);
		fclose(pFile);
	}
	//free(Buff);
}
void seperate(){
	string fileName;
	int cWord = 0;
	FILE * pFile;
	char anyBuffer[1000];
	string oNameFile(FILE_SEPERATE_NAME);
	ofstream oFile;
	register unsigned int fLen;
	int numSep=0;
	oNameFile = oNameFile + _itoa(numSep,anyBuffer,10)+ ".txt";
	oFile.open(oNameFile, ios::app | ios::out);
	unsigned char * buff = new unsigned char[MaxSize + 1];
	for (curFile = 0; curFile < numFile; ++curFile){
		//buff = buffer[curFile];
		fileName = FILENAME;
		fileName = fileName + _itoa(curFile+1, anyBuffer, 10) + ".txt";
		pFile = fopen(fileName.c_str(), "rb");
		fLen = fileLen[curFile];
		fread(buff, fLen, 1, pFile);
		buff[fLen] = '\0';	
		register unsigned int j = 0;
		unsigned char tmp;
		int cSep = 0;
		for (register unsigned int i = 0; i <= fLen; ++i){
			tmp = buff[i];
			oFile << buff[i];
			if ((tmp >= 97 && tmp <= 122) || (tmp >= 65 && tmp <= 90)){
				++j;
			}
			else if (j>0) {
				if (++cWord >= 50000){
					cWord = 0;
					++cSep;
					oFile.close();
					oNameFile = FILE_SEPERATE_NAME;
					oNameFile = oNameFile + _itoa(++numSep,anyBuffer,10) + ".txt";
					oFile.open(oNameFile, ios::app | ios::out);
					if (cSep >= 7) {
						cSep = 0;
						break;
					}
				}
				j = 0;
			}
		}
		fclose(pFile);
	}
	oFile.close();
	delete[] buff;

}
/*
void Search(Trie * trie){
	FILE * pFile;
	
	string fileName = SEARCH_FILE_NAME;
	register unsigned int fLen, result = 0;
	pFile = fopen(fileName.c_str(), "rb");
	fseek(pFile, 0, SEEK_END);
	fLen = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	unsigned char * buff = new unsigned char[fLen+ 1];
	fread(buff, fLen, 1, pFile);
	buff[fLen] = '\0';

	FILE* fout=fopen(SEARCH_RESULT_FILE_NAME,"wt");
	register unsigned int j = 0, tmp, upcase = 32;
	bool err=0,noNode=0;
	clock_t start, end;
	double total=0;

	char word[100000],realWord[100000];
	for (register unsigned int i = 0; i <= fLen; ++i){
		tmp = buff[i];
		if ((tmp >= 97 && tmp <= 122) || (tmp >= 65 && tmp <= 90)){
				realWord[j]=tmp;
				tmp = (tmp | upcase) & (-97);
				word[j++]=tmp;
		}
		else if (48<=tmp && tmp<=57){
				realWord[j]=tmp;	
				tmp -=21;
				word[j++]=tmp;
		}
		else if (j > 0) {
			start=clock();
			realWord[j]=0;
			Node* newNode=trie->search(word,j);
			end=clock();
			fprintf(fout,"%s : ",realWord);
			noNode=false;
			if (newNode)
			for (int i = 0; i < numFile; ++i)
				if (newNode->nFile[i]!=0) {fprintf(fout,"%d ",i); noNode=true;}		
			if (noNode && (newNode)) fprintf(fout, "\n"); else fprintf(fout, "NULL\n");
			
			total+=start-end;
			if (tmp == 13) ++i;
			j = 0;
		}
	}
	
	printf("%d\n",total);
	fclose(pFile);
	fclose(fout);
	delete[] buff;
}
*/
bool* calculate(list<bool*> & query , list<bool> & Operator ){
	list<bool>::iterator it0=Operator.begin();
	list<bool*>::iterator it1=query.begin();
	bool* tmp1,*tmp2;
	while ( it0!= Operator.end()){
		if ((*it0)==1){
			tmp1=*it1;
			it1++;
			tmp2=*it1;
			for(int i=0;i<numFile;++i)
				(*it1)[i]&=tmp1[i];
			it1--;
			delete[](*it1);
			it1=query.erase(it1);
			it0=Operator.erase(it0);
		}
		else {
			++it0;
			++it1;
		}
	}
	it0=Operator.begin();
	it1=query.begin();
	while ( it0!= Operator.end()){
			tmp1=*it1;
			it1++;
			tmp2 = *it1;
			for(int i=0;i<numFile;++i)
				(*it1)[i]|=tmp1[i];
			it1--;
			delete[](*it1);
			it1=query.erase(it1);
			it0=Operator.erase(it0);
	}
	return *query.begin();
}
bool PrintSubTrie(Node* root, const char* word, FILE * fout){
	if (!root) {
		fprintf(fout, "NULL\n");
		return false ;
	}
	bool sDone=false;
	stack <Node*> St;  St.push(root);
	stack <string> trace; trace.push(string(word));
	Node* current;
	string curStr,prevStr;
	clock_t start, end;
	start = clock();
	bool isWrite = false;
	while (St.empty() == false){
		current = St.top();
		prevStr = trace.top();
		St.pop();
		trace.pop();
		isWrite = false;
		for (int i = 0; i < numFile; ++i){
			if (current->nFile[i]){
				end = clock();
				int time = end - start;
				totalSearchTime += time;
				if (!isWrite) fprintf(fout, "%s || Time : %d ms -> ", prevStr.c_str(), time);
				isWrite = 1;
				fprintf(fout, "%s - ", FileName[i]);
				start = clock();
			}
		}
		if (isWrite) {
			sDone = true;
			fprintf(fout, "\n");
		}
		for (int i = 1; i < _MAXCHAR;++i) 
		if (current->mChildren[i]){
			if (i<27) curStr = prevStr+ char(i + 96);
			else curStr = prevStr + char(i + 21);
			St.push(current->mChildren[i]);
			trace.push(curStr);
		}
	}
	return sDone;
}
bool PrintUpTrie(Node* root, char* Rword,char* word,int len, FILE * fout){
	stack <Node*> St;  St.push(root);
	stack <string> trace; trace.push("");
	Node* current;
	string curStr, prevStr;
	clock_t start, end;
	start = clock();
	bool isWrite,sDone=false;
	Node* tmp = NULL;
	while (St.empty() == false){
		current = St.top();
		prevStr = trace.top();
		St.pop();
		trace.pop();
		tmp = Trie::searchFromNode(current, word, len);
		if (tmp){
			end = clock();
			int time = end - start;
			totalSearchTime += time;
			isWrite = false;
			for (int i = 0; i < numFile; ++i){
				if (tmp->nFile[i]){
					if (!isWrite) fprintf(fout, "%s%s || Time %d ms -> ", prevStr.c_str(), Rword,time);
					isWrite = 1;
					fprintf(fout, "%s - ", FileName[i]);
				}
			}
			if (isWrite){ 
				sDone = true; 
				fprintf(fout, "\n"); 
			}
			start = clock();
		}
		for (int i = 1; i < _MAXCHAR; ++i){
			tmp = current->mChildren[i];
			if (tmp){
				if (i<27) curStr = prevStr + char(i + 96);
				else curStr = prevStr + char(i + 21);
				St.push(tmp);
				trace.push(curStr);
			}
		}
	}
	return sDone;
}
bool SCase(Trie* trie, Node* root, char* Rword, char* word, int len, FILE * fout){
	stack <Node*> St;  St.push(root);
	stack <string> trace; trace.push("");
	Node* current;
	string curStr, prevStr;
	bool sDone=false ;
	Node* tmp = NULL;
	while (St.empty() == false){
		current = St.top();
		prevStr = trace.top();
		St.pop();
		trace.pop();
		tmp = Trie::searchFromNode(current, word, len);
		if (tmp){
			curStr = prevStr + Rword;
			sDone=PrintSubTrie(tmp, curStr.c_str(), fout);
		}
		for (int i = 1; i < _MAXCHAR; ++i){
			tmp = current->mChildren[i];
			if (tmp){
				if (i<27) curStr = prevStr + char(i + 96);
				else curStr = prevStr + char(i + 21);
				St.push(tmp);
				trace.push(curStr);
			}
		}
	}
	return sDone;
}

void Search(Trie * trie,FILE* fout){
	FILE * pFile;
	bool* result=NULL;
	static bool def[1000];
	string fileName = SEARCH_FILE_NAME;
	pFile = fopen(fileName.c_str(), "rb");
	char str[100000],word[100000],realWord[100000],endline[1000];
	memset(endline, '-', 999);
	endline[999] = 0;
	int j=0,len=0;
	bool scase = 0 ;
	char* pch=NULL, *p=NULL;
	int numquery = ++numQuery;
	while (--numquery){
		fgets(str, 100000, pFile);
		len = strlen(str);
		p = strstr(str, "\r");
		if (p == NULL){ 
			str[len++] = '\r'; str[len++] = '\n';
			str[len] = 0;
		}
		pch = strstr (str,"*");
		fprintf(fout, "Query: %s", str);
		if (pch!=NULL){	
			bool sDone = 0;
			pch = strstr(pch+1, "*");
			if (pch == NULL) scase = 1;
			for (int i = 0; i < len; ++i){
				if (str[i] == 13){
					realWord[j] = '\0';
					if (!scase){
						scase = 0; 
						sDone=SCase(trie, trie->root, realWord, word, j, fout);
					}
					else if (str[0] != '*') sDone=PrintSubTrie(trie->search(word, j), realWord, fout);
					else  sDone = PrintUpTrie(trie->root, realWord, word, j, fout);
					j = 0;
				}
				else{
					char tmp = str[i];
					if ((tmp >= 97 && tmp <= 122) || (tmp >= 65 && tmp <= 90)){
						realWord[j] = tmp;
						tmp = (tmp | 32) & (-97);
						word[j++] = tmp;
					}
					else if (48 <= tmp && tmp <= 57){
						realWord[j] = tmp;
						tmp -= 21;
						word[j++] = tmp;
					}
				}
			}
			
			if (!sDone) fprintf(fout, " NULL\n");
			fprintf(fout,"%s\n", endline);

		}
		else{
			clock_t start, end;
			start = clock();
			list<bool*> query;
			list<bool> Operator;
			Node* newNode=NULL;
			bool isRela=1;
			for (int i=0;i<len;++i)
				if (str[i]==' '||str[i]==13){
					newNode = trie->search(word,j);
					result = new bool[numFile];
					if (newNode) 
						for (int j = 0; j < numFile; ++j) result[j] = (newNode -> nFile[j]>0);
					else
						for (int j = 0; j < numFile; ++j) result[j] = 0;
					query.push_back(result);
					j=0;
					if (str[i+1]=='A'&&str[i+2]=='N'&&str[i+3]=='D') {
						isRela=0;	
						Operator.push_back(1);
						i+=4;
					}
					else if (str[i+1]=='O'&&str[i+2]=='R'){
						isRela=0;
						Operator.push_back(0);
						i+=3;
					}
				}
				else{
					char tmp = str[i]; 
					if ((tmp >= 97 && tmp <= 122) || (tmp >= 65 && tmp <= 90)){
						tmp = (tmp | 32) & (-97);
						word[j++]=tmp;
					}
					else if (48<=tmp && tmp<=57){
						tmp -=21;
						word[j++]=tmp;
					} 
				}
				
				if (!isRela) {
					if (query.size() == Operator.size()){
						newNode = trie->search(word, j);
						result = new bool[numFile];
						if (newNode)
						for (int j = 0; j < numFile; ++j) result[j] = (newNode->nFile[j]>0);
						else
						for (int j = 0; j < numFile; ++j) result[j] = 0;
						query.push_back(result);
					}
					result = calculate(query, Operator);
				}
				end = clock();
				int time = end - start;
				totalSearchTime += time;
				fprintf(fout, "|| Time %d ms ->", time);
				bool noNode = false;
				if (newNode != NULL){
					if (isRela){
						for (int i = 0; i < numFile; ++i)
						if (result[i] != 0) { fprintf(fout, "\n%s - %d times ", FileName[i], newNode->nFile[i]); noNode = true; }
					}
					else
					for (int i = 0; i < numFile; ++i)
						if (result[i] != 0) { fprintf(fout, "%s - ", FileName[i]); noNode = true; }
				}
				else noNode = true;
				if (noNode && (result)) fprintf(fout, "\n"); else fprintf(fout, " NULL\n");
				fprintf(fout, "%s\n", endline);
				delete[] result;
		}
	}
	fclose(pFile);
}
void StableProcess(){
	for (int i = 0; i < 3; ++i){
		Trie* trie = new Trie();
		Buff = new unsigned char[MaxSize + 2];
		inputTxT_Build(trie);
		delete[] Buff;
		trie->Destroy();
	}
}
int main()
{
	//open file
	if (!MODE) seperate();
	else{
		printf("ConStruct time:\n");
		clock_t start, end;
		Trie* trie;
		StableProcess();
		double time, sumTime = 0;
		for (int i = 0; i < 50; ++i){
			trie = new Trie();
			Buff = new unsigned char[MaxSize + 2]; 
			start = clock();
			//input
			inputTxT_Build(trie);
			end = clock();
			time = (double)(end - start);
			delete[] Buff;
			
			printf("Time: %lf\n",time);
			sumTime += time;
			if (i!=49) trie->Destroy();
		}

		printf("\t\t\tTotal Time: %lf\n",sumTime / 50);
		FILE* fout = fopen(SEARCH_RESULT_FILE_NAME, "wt");
		fprintf(fout, "**** Searching query ****\n");
		Search(trie,fout);
		fclose(fout);
		printf("\t\t\tSearch Time:%d \n", totalSearchTime);
		//printf("\t\t\tTotal Node in Trie %d\n",currentBlock);
		fout = fopen(SEARCH_RESULT_FILE_NAME, "a");
		fprintf(fout, "------------SUMARY-------------------------------------------------------------------------------------------------------------\n");
		fprintf(fout,"Inputing %d file data : \n", numFile);
		fprintf(fout, "Total Size: %lf  MBs  \n", totalSize/(1024*1024));
		fprintf(fout,"Average Time PreProcessor: %lf  ms\n", sumTime / 50);
		fprintf(fout,"Total Search Time:%d ms  \n", totalSearchTime);
		fprintf(fout, "-------------------------------------------------------------------------------------------------------------------------------\n");
		fclose(fout);
		delete trie;
    	}
	getchar();
	return 0;
}
