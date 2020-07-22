#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "kabu.h"

#define Pause 0.5

void readstocks(void);                    // データファイルの読込(暗号化されている）
void setprice(int date, int* price);        // 当日の株価の取得（暗号化解除）
void trading(int date, int stocklist);    // 取引プログラム（経過日数，保有銘柄数）

int sell(int date, int stock, int share, int s_price);    // 売り注文（経過日数，銘柄，株数，指値）←成行きの場合は指値＝NARI
int buy(int date, int stock, int share, int b_price);        // 買い注文（経過日数，銘柄，株数，指値）←成行きの場合は指値＝NARI

int getaccount(void);                    // 買い付け余力情報の受け渡し
int getproperty(void);                // 資産総額の受け渡し
int getstocklist(void);                // 保有銘柄数の受け渡し
void getportfolio(int* t_portfolio);    // ポートフォリオ情報の受け渡し
int gethprice(int stock);            // 特定銘柄の平均取得単価の受け渡し

// 一般公開情報（市場が公開している情報）
char code[STOCKS][5];        // 証券コード番号
char name[STOCKS][31];        // 銘柄名
int pricelog[LOG][STOCKS];    // 過去の株価データ


// 個人情報（証券会社が管理しており，ユーザからの要請で情報提供するもの）
// ＃構造体を用いて管理するのがベストでしょう．


int account = ACCOUNT;        // 口座残高（ACCOUNTは初期値）
int property = ACCOUNT;        // 資産総額
int portfolio[STOCKS];        // 保有株リスト（株数）
int hprice[STOCKS];            // 平均取得単価
int stocklist = 0;            // 保有銘柄数











/********************************************************
    株の自動売買プログラム（証券会社側）

    ユーザの個人情報管理，売買注文の受付，約定通知を
    行うプログラム
********************************************************/

int main(void)
{
    
    extern int pricelog[LOG][STOCKS];    // 過去の株価データ
    extern int portfolio[STOCKS];        // 保有株リスト（株数）
    extern int stocklist;                // 保有銘柄数
    extern int account;                    // 口座残高
    extern int property;                // 資産総額
     
    int price[STOCKS];    // 株価
    int date;            // 経過日数

    int i;

    printf("kabumachine start!\n");

    // ポートフォリオの初期化（最初は保有株数０）
    for (i = 0; i < STOCKS; i++)
    {
        portfolio[i] = 0;
    }

    // 取扱銘柄，過去および未来の株価の読込
    readstocks();

    printf("Load stocks completed!\n\n");

    //sleep(1 * 1000 * Pause);

    /******************************************************************
        過去ログの取得（株価ファイルの先頭LOG日分を過去ログにコピー）

        過去ログ配列pricelog[n]には，取引日のn+1日前の日の株価が
        格納されています．
    *******************************************************************/

    for (i = LOG - 1; i >= 0; i--)
    {
        setprice(i + 1, pricelog[LOG - 1 - i]);        //    元はsetprice(i-LOG, pricelog[LOG-1-i]);
    }

    printf("Have a good deal!\n\n");

    //sleep(2 * 1000 * Pause);

    /********************************************************
        取引開始
    ********************************************************/

    date = 1; // 取引１日目

    while (date <= DATE)
    {
        printf("*******************************************************\n\n");
        printf(" %d日目\n", date);
        //printf(" %d日目¥n", date);

        trading(date, stocklist);// 売買注文の受付

        setprice(date + LOG, price);// 当日の株価の取得
        property = account;
        for (i = 0; i < STOCKS; i++)
        {
            property += portfolio[i] * price[i];// 資産総額の更新
        }

        // 株価ログの更新
        int j;//ループに用いる変数
        for (i = 0; i < STOCKS; i++)
        {
            for (j = LOG - 1; j >= 1; j--)
            {
                pricelog[j][i] = pricelog[j - 1][i];
            }
            pricelog[0][i] = price[i];
        }

        printf(" 口座残高 : %d\n", account);
        printf(" 持ち株数 : %d\n", stocklist);
        printf(" 総資産 : %d\n\n", property);

        //sleep(5 * 100 * Pause);

        date++;
    }
    /***********************************************************
    exeによる出力結果表示のための一時停止用入力
    本番実行時に必要なので意味のない関数に見えますが消さないでください
    ************************************************************/
    int q;
    scanf("%d", &q);//exeによる出力結果表示のための一時停止用入力
}



/****************************************************************
    以下，売買注文のプログラムを作成します．

    これらの関数は，トレーディングプログラムから呼び出されます．
    買い注文の場合は，ユーザの口座残高を超えた買い注文になって
    いないか確認する必要があります．
    売り注文の場合は，ユーザが保有している株かどうかを確認する
    必要があります．

    いずれの関数も，売買が成立した場合に約定単価g_priceを返します．
    売買不成立の場合，g_price=ERRORを返します．
    ERRORはkabu.hで定義されています．
*****************************************************************/


int sell(int date, int stock, int share, int s_price)
{
    int g_price; // 売り約定単価
    int price[STOCKS];

    // 当日の株価の取得
    setprice(date + LOG, price);

    // 成り売りの場合
    if (s_price == NARI)
        g_price = price[stock];

    // 指値売り注文の場合
    else if (s_price <= price[stock])
        g_price = s_price;
    else
        g_price = ERROR;

    // 持ち株が売り注文数未満の場合
    if (portfolio[stock] < share)
        g_price = ERROR;

    // 約定が成立した場合
    if (g_price > ERROR)
    {
        //口座に振込
        account += g_price * share;

        //ポートフォリオを更新
        portfolio[stock] -= share;
        stocklist -= share;

        // 約定通知
        printf(" [[売り約定]] : 株 %d, 数 %d, 売り単価 %d\n", stock, share, g_price);
        printf("-----------------------------------\n");
    }
    else
    {
        // 約定不成立通知
        printf("  売り不成立  : 株 %d, 数 %d, ！持ち株不足！\n", stock, share);
        g_price = ERROR;
        printf("-----------------------------------\n");
    }

    return(g_price);
}


int buy(int date, int stock, int share, int b_price)
{
    int g_price; // 買い約定単価
    int price[STOCKS];

    // 当日の株価の取得
    setprice(date + LOG, price);

    // 成り買いの場合
    if (b_price == NARI)
        g_price = price[stock];

    // 指値買い注文の場合
    else if (b_price >= price[stock])
        g_price = b_price;
    else
        g_price = ERROR;

    // 口座残高が注文額未満の場合
    if (account < g_price * share)
        g_price = ERROR;

    // 約定が成立した場合
    if (g_price > ERROR)
    {
        account -= g_price * share;

        // 平均取得単価の更新
        hprice[stock] = (int)((hprice[stock] * portfolio[stock] + g_price * share) / (portfolio[stock] + share));

        //ポートフォリオを更新
        portfolio[stock] += share;
        stocklist += share;

        // 約定通知
        printf(" [[買い約定]] : 株 %d, 数 %d, 買い単価 %d\n", stock, share, g_price);
        printf(" 更新後の平均取得単価 : %d\n", hprice[stock]);
        printf("-----------------------------------\n");
    }
    else
    {
        // 約定不成立通知
        printf("  買い不成立  : 株 %d, 数 %d, ！残高不足！\n", stock, share);
        g_price = ERROR;
        printf("-----------------------------------\n");
    }

    return(g_price);
}




/*************************************************************************
    以下，個人情報呼び出し関数を記述してください．
    これらは，トレーディングプログラムが売買を行う際に，自分の口座情報や
    保有株の情報など参考にするために用意されるものです．

    トレーディングプログラムは，自分の口座残高を確認し，限度額を超えない
    ように買い注文を出さなくてはなりません．また，保有銘柄以外の銘柄に
    売り注文を出すことはできません（カラ売り不可）

    したがって，売買注文を出す場合に，以下の関数により個人情報を取得し，
    ルールに従った売買を行う（sell,buy関数を呼ぶ）必要があります．


    ＃構造体を使用した場合は，関数も構造体に対応する形でまとめてください
**************************************************************************/

// 口座残高の確認

int getaccount(void)
{
    //extern int account;    // 口座残高

    return(account);
}


// 資産総額の確認

int getproperty(void)
{
    //extern int property;    // 資産総額

    return(property);
}


// 保有銘柄数の確認

int getstocklist(void)
{
    //extern int stocklist;    // 保有銘柄数

    return(stocklist);
}


// ポートフォリオの確認

void getportfolio(int* t_portfolio)
{
    //extern int portfolio[STOCKS];    // ポートフォリオ

    int i;

    for (i = 0; i < STOCKS; i++)
    {
        t_portfolio[i] = portfolio[i];
    }
}


//　任意銘柄の平均取得単価の確認

int gethprice(int stock)
{
    //extern int hprice[STOCKS];    // 平均取得単価

    int t_price;

    t_price = hprice[stock];

    return(t_price);
}
