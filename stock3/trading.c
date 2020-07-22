#include <stdio.h>
#include <stdlib.h>
#include "kabu.h"

#define SPAN 5             //移動平均線の期間
#define LOGSIZE 5           //移動平均線ログのサイズ


// 一般公開情報（市場が公開している情報）
extern char code[STOCKS][5]; // 証券コード番号
extern char name[STOCKS][31]; // 銘柄名
extern int pricelog[LOG][STOCKS]; // 過去の株価データ


double average[STOCKS];       //移動平均線　その日の銘柄の平均値
double trendlog[LOGSIZE][STOCKS];          // 過去の移動平均線のデータ


// 注文
int sell( int date, int stock, int share, int s_price );    // 売り注文（経過日数，銘柄，株数，指値）←成行きの場合は指値＝NARI
int buy( int date, int stock, int share, int b_price);        // 買い注文（経過日数，銘柄，株数，指値）←成行きの場合は指値＝NARI

// 個人情報の呼び出し（構造体を使用している場合は，適当に変更してください）
int getaccount( void );                        // 買い付け余力の確認
int getproperty( void );                    // 資産総額の確認
int getstocklist( void );                    // 保有銘柄数の確認
void getportfolio( int *t_portfolio );        // ポートフォリオの確認
int gethprice( int stock );                    // 特定銘柄の平均取得株価の確認



int movingsum[STOCKS];      //各銘柄のLOG日分の株価の合計
double gap;

/********************************************************
    株の自動売買プログラム（ユーザ側）

    特定の条件を満たす銘柄について，売り買い注文を
    自動的に行うプログラム
********************************************************/


void trading( int date, int stocklist )
{
    int my_portfolio[STOCKS];    // ポートフォリオ
    int my_account;                // 口座残高
    int my_property;            // 資産総額
    int my_stocklist;            // 保有銘柄数
    //　↑構造体で管理する場合は，適当に変更してください
    //    user_info my_info; など

    int i, j;
    
    

    /********************************************************

            個人情報の取得（必要があれば）

    ********************************************************/

    getportfolio(my_portfolio);
    my_account = getaccount();        // 特にここで呼ぶ必要なし
    my_property = getproperty();    // 特にここで呼ぶ必要なし
    my_stocklist = getstocklist();
    
    
    
    //移動平均線のログの初期化
    
    if(date == 1)
    {
        for(i=0;i<LOGSIZE;i++)
        {
            for(j=0;j<STOCKS;j++)
            {
                trendlog[i][j] = 0;
            }
        }
    }
    
    
    
    
    //当日の移動平均値の取得
    
    for(i=0;i<STOCKS;i++)
    {
        movingsum[i] = 0;
        for(j=0;j<SPAN;j++)
        {
            movingsum[i] += pricelog[j][i];
        }
        //printf("%d\n", pricelog[0][i]);
        average[i] = (double)movingsum[i]/SPAN;
        //printf("%d: %lf\n", i, average[i]);           //当日の移動平均線

    }
    
    
    
    
    //過去の移動平均線のログの取得
    
    for(i=LOGSIZE-1;i>=0;i--)
    {
        if(i==0)
        {
            for(j=0;j<STOCKS;j++)
            {
                trendlog[i][j] = average[j];
            }
        }else
        {
            for(j=0;j<STOCKS;j++)
            {
                trendlog[i][j] = trendlog[i-1][j];
            }
        }
    }

    
    
    
    
    /********************************************************

            売り注文

    ********************************************************/

    int s_price;                    //売り約定値

    if (my_stocklist > 0) // 株を保有している場合
    {
        for (i = 0; i < STOCKS; i++)
        {
            if (my_portfolio[i] != 0) // 銘柄iを保有している場合
            {
                /***********************************************

                　売り注文プログラム（ここにオリジナルを作成）

                ************************************************/
                
                //printf("株価：%d  平：%lf\n", pricelog[0][i], average[i]);         //test
                
                //最終日に全銘柄売り
                if(date==DATE)
                {
                    s_price = sell(date, i, my_portfolio[i], NARI);
                }else
                {
                    //株価が移動平均線を下側に抜けた場合
                    if(pricelog[1][i]>=trendlog[1][i] && pricelog[0][i]<trendlog[0][i])
                    {
                        s_price = sell(date, i, my_portfolio[i], NARI);
                        
                    }
                    else if(pricelog[1][i]>=average[i] && pricelog[0][i]<average[i])
                    {
                        s_price = sell(date, i, my_portfolio[i], NARI);
                    }
                }


                /************************************************
                        ここまでが売り注文プログラム
                *************************************************/

            }
        }
    }



    /********************************************************

            買い注文

    ********************************************************/

    int money;                            // 買い付け予算

    

    /*************注文処理*************/
    my_account = getaccount(); // 口座残高の取得
    money = my_account; // 買い付け余力確認（現状では，口座残高＝買付け余力）
    
    int b_price; // 約定単価

    for(i=0;i<STOCKS;i++)
    {
        //printf("株価：%d  平：%lf\n", pricelog[0][i], average[i]);         //test
        
        if (money >= pricelog[0][i] * 1000)
        {
            // 銘柄iを保有していない場合
            if(my_portfolio[i] == 0)
            {
                //初日に平均より上だったら買い
                if(date==1)
                {
                    if(pricelog[0][i]>=average[i])
                    {
                        b_price = buy(date, i, 1000, NARI);  // 購入単価の取得（エラーの場合はERROR=-1）
                    }
                }//最終日は買わない
                else if(date!=DATE)
                {
                    //株価が移動平均線を上側に抜けた場合1
                    if(pricelog[2][i]<=trendlog[2][i] && pricelog[1][i]>=trendlog[1][i] && pricelog[0][i]>trendlog[0][i] )
                    {
                        b_price = buy(date, i, 1000, NARI);  // 購入単価の取得（エラーの場合はERROR=-1）
                        
                    }//株価が移動平均線を上側に抜けた場合2
                    else if(pricelog[1][i]<=average[i] && pricelog[0][i]>average[i])
                    {
                        b_price = buy(date, i, 1000, NARI);  // 購入単価の取得（エラーの場合はERROR=-1）

                    }
                }
            }
            
        }
    }
    

    /************************************************
            ここまでが買い注文プログラム
    *************************************************/
    
    
}

