#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kabu.h"

void readstocks(void);
char* setvalue(char* s, char* t, int size);
void setprice(int date, int* price);


extern char code[STOCKS][5];
extern char name[STOCKS][31];
 

static char pricedata[DATE + LOG][STOCKS][5];// 過去＋未来の株価データ（外部からの参照禁止）




/*******************************************************************************

  過去および未来の株価データを株価ファイルから読み込む関数

********************************************************************************/

void readstocks(void)
{
    int i, j;//ループ変数
    //ファイルオープン処理
    FILE* fp;
    fp = fopen("kabu_b.csv", "r");/*ファイルオープン*/
    if (fp == NULL)
    {
        printf("Open Error\n");
    }
    else/*ファイル読み込み*/
    {
        //コードの読み込み
        for (i = 0; i < STOCKS; i++)
        {
            if (i == STOCKS - 1)
                fscanf(fp, "%[^\n]", code[i]);
            else
                fscanf(fp, "%[^,]", code[i]);
            fgetc(fp);
        }

        //銘柄名の読み込み
        for (i = 0; i < STOCKS; i++)
        {
            if (i == STOCKS - 1)
                fscanf(fp, "%[^\n]", name[i]);
            else
                fscanf(fp, "%[^,]", name[i]);
            fgetc(fp);
        }

        //株価の読み込み（DATE+LOG日分の株価データを順番に読み込みます）
        for (j = 0; j < DATE + LOG; j++)
        {
            for (i = 0; i < STOCKS; i++)
            {
                if (i == STOCKS - 1)
                    fscanf(fp, "%[^\n]", pricedata[j][i]);
                else
                    fscanf(fp, "%[^,]", pricedata[j][i]);

                fgetc(fp);
            }
        }
    }
}

/*******************************************************************************

  文字列コピー関数（演習で使用した文字列コピー関数です．不要なら削除してください）

********************************************************************************/

char* setvalue(char* s, char* t, int size)
{
    int   i;

    // ","出現まで一文字ずつコピー
    for (i = 0; i < size && *s != ','; i++, s++)
    {
        *t++ = *s;
    }

    // 末尾にヌルを追加
    *t = '\0';

    return(s + (*s == ','));
}

/*******************************************************************************

  1日の株価を返す関数

  mainプログラムから呼び出すことで，当日分の株価を取得します．

********************************************************************************/

void setprice(int date, int* price)
{

    /**********************************************
        readstocksで読み込んだ株価データのdate日
        の株価をSTOCKS分取得します．
        readstocks内では株価はchar型なので，intに
        変換して値を返します．
    ************************************************/
    int i;
    for (i = 0; i < STOCKS; i++)
    {
        *(price + i) = atoi(pricedata[date - 1][i]);//int型に変換
    }
}
