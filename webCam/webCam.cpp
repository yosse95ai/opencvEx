#include<iostream>                              //stdの利用
#include<unordered_map>                         //順序なし連想配列の利用
#include<vector>                                //Vector型の利用
#include <time.h>                               // 実行時間計測用
#include <stdio.h>
#include <windows.h>
#include<opencv2/opencv.hpp>                    //OprnCVの利用

#define GREEN_FLOOR_SIZE 200                    // 領域の最小面積
#define GREEN_FLOOR cv::Scalar(30, 100, 0)      // 緑色範囲1
#define GREEN_UPPER cv::Scalar(90, 255, 255)    // 緑色範囲2

typedef std::unordered_map<std::string, double> dict;   // 辞書型の宣言(順序無し)
typedef std::unordered_map<int, dict> shelf;            // 棚型の宣言(辞書を保有する辞書)

// 関数宣言
cv::Mat green_detect(cv::Mat frame);            // 二値化 (緑色領域: 255, その他: 0)
std::vector<dict> blob_anarysis(cv::Mat frame); // ブロブ解析

// ラッププリント関数
template <class C>
void print(const C& c, std::ostream& os = std::cout)
{
    std::for_each(std::begin(c), std::end(c), [&os](typename C::value_type p) { os << '{' << p.first << ',' << p.second << "}, "; });
    os << std::endl;
}

// 緑色領域の抽出
cv::Mat green_detect(cv::Mat frame)
{
    cv::Mat hsv;

    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

    cv::inRange(hsv, GREEN_FLOOR, GREEN_UPPER, frame);

    return frame;
}

// ブロブ解析
std::vector<dict> blob_anarysis(cv::Mat frame)
{
    cv::Mat labels, stats, centroids;           // 情報の登録先
    int nLab = cv::connectedComponentsWithStats(frame, labels, stats, centroids);   // 情報取得
    shelf label_info;
    std::vector<int> areas = {};

    // ラベリング情報の登録
    int map_index = 0;                          // 解析情報登録用インデックス
    for (int i = 1; i < nLab; ++i)              // すべての情報を参照していく
    {
        int* statsPtr = stats.ptr<int>(i);              // stats用ポインタ
        //double* centerPtr = centroids.ptr<double>(i);   //centroids用ポインタ
        int area = statsPtr[cv::ConnectedComponentsTypes::CC_STAT_AREA];

        if (area > GREEN_FLOOR_SIZE)            // GREEN_FLOOR_SIZEより小さな領域は無視
        {
            dict detail { 
                {"left", statsPtr[cv::ConnectedComponentsTypes::CC_STAT_LEFT] },
                {"top", statsPtr[cv::ConnectedComponentsTypes::CC_STAT_TOP] },
                {"height", statsPtr[cv::ConnectedComponentsTypes::CC_STAT_HEIGHT] },
                {"width", statsPtr[cv::ConnectedComponentsTypes::CC_STAT_WIDTH] },
                {"area", area },
                //{"center_x", static_cast<int>(centerPtr[0]) },
                //{"center_y", static_cast<int>(centerPtr[1]) }
            };
            areas.push_back(area);
            label_info.emplace(map_index, detail);
            map_index++;
        }            
    }

    std::vector<dict> ret;
    // 2個以下の最大値を抽出
    if (areas.size() > 1)
    {
        //std::cout << "gets!!-----------------------" << std::endl;
        ret.push_back(dict{ { "FLAG", 2 } });   // フラグ(抽出個数)
        // 1個目の最大領域
        auto detail_itr = std::max_element(areas.begin(), areas.end());
        int max_index = std::distance(areas.begin(), detail_itr);
        ret.push_back(label_info[max_index]);  // 情報の登録
        areas[max_index] = 0;              // 1個目の最大領域の情報を削除
        // 2個目の最大領域
        detail_itr = std::max_element(areas.begin(), areas.end());
        max_index = std::distance(areas.begin(), detail_itr);
        ret.push_back(label_info[max_index]);  // 情報の登録
    }
    else if (areas.size() > 0)
    {
        //std::cout << "get!" << std::endl;
        ret.push_back(dict{ { "FLAG", 1 } });   // フラグ(抽出個数)
        auto detail_itr = std::max_element(areas.begin(), areas.end());
        auto label_info_itr = label_info.begin();
        int max_index = std::distance(areas.begin(), detail_itr);
        ret.push_back(label_info[max_index]);  // 情報の登録
    }
    else
    {
        //std::cout << "none" << std::endl;
        ret.push_back(dict{ { "FLAG", 0 } });   // フラグ(抽出個数)
    }

    return ret;
}

std::vector<dict> blob_anarysis2(cv::Mat frame)
{
    cv::Mat labels, stats, centroids;           // 情報の登録先
    int nLab = cv::connectedComponentsWithStats(frame, labels, stats, centroids);   // 情報取得

    std::vector<cv::Mat> stat_ = {};
    std::vector<int> areas = {};

    // ラベリング情報の登録
    for (int i = 1; i < nLab; ++i)              // すべての情報を参照していく
    {
        int* statsPtr = stats.ptr<int>(i);              // stats用ポインタ
        //double* centerPtr = centroids.ptr<double>(i);   //centroids用ポインタ
        int area = statsPtr[cv::ConnectedComponentsTypes::CC_STAT_AREA];

        if (area > GREEN_FLOOR_SIZE)            // GREEN_FLOOR_SIZEより小さな領域は無視
        {
            stat_.push_back(stats.row(i));
            areas.push_back(area);
        }
    }
    std::vector<dict> ret = {  };
    int loop = 0;
    // 2個以下の最大値を抽出
    if (areas.size() > 1)
    {
        loop = 2;
        ret.push_back({ {"FLAG", 2} });
    }
    else if (areas.size() > 0)
    {
        loop = 1;
        ret.push_back({ {"FLAG", 1} });    
    }
    else
    {
        ret.push_back({ {"FLAG", 0} });
        return ret;
    }
    for (int i = 0; i < loop; i++)
    {
        auto detail_itr = std::max_element(areas.begin(), areas.end());
        int max_index = std::distance(areas.begin(), detail_itr);
        int* max_stats = stat_[max_index].ptr<int>(0);
        ret.push_back({ 
                {"left", max_stats[cv::ConnectedComponentsTypes::CC_STAT_LEFT] },
                {"top", max_stats[cv::ConnectedComponentsTypes::CC_STAT_TOP] },
                {"height", max_stats[cv::ConnectedComponentsTypes::CC_STAT_HEIGHT] },
                {"width", max_stats[cv::ConnectedComponentsTypes::CC_STAT_WIDTH] },
                {"area", areas[max_index] }
            } );
        areas[max_index] = 0;

    }
    return ret;
}

void debug1()//関数化バージョン
{
    //＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊ QueryPerformanceCounter関数の1秒当たりのカウント数を取得する
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    LARGE_INTEGER start, end;
    QueryPerformanceCounter(&start);
    // ここから計測

    //画像を入力
    cv::Mat frame1 = cv::imread("C:\\Users\\surfa\\Desktop\\originalExe\\image\\dora.jpg");
    //cv::Mat frame2 = cv::imread("C:\\Users\\surfa\\Desktop\\originalExe\\image\\HLSColorSpace.png");
    cv::Mat frame = frame1;

    //二値化処理
    cv::Mat green = green_detect(frame);

    std::vector<dict> target = blob_anarysis2(green);

    if (target[0]["FLAG"] == 2)
    {
        dict stick1 = target[1];
        dict stick2 = target[2];
        cv::rectangle(frame, cv::Rect((int)stick1["left"], (int)stick1["top"],
            (int)stick1["width"], (int)stick1["height"]), cv::Scalar(255, 0, 255), 2);
        cv::rectangle(frame, cv::Rect((int)stick2["left"], (int)stick2["top"],
            (int)stick2["width"], (int)stick2["height"]), cv::Scalar(255, 0, 255), 2);
    }
    else if (target[0]["FLAG"] == 1)
    {
        cv::rectangle(frame, cv::Rect((int)target[1]["left"], (int)target[1]["top"], 
            (int)target[1]["width"], (int)target[1]["height"]), cv::Scalar(255, 0, 255), 2);
    }

    cv::imshow("plane", frame);
    cv::imshow("green", green);
    // ここまでの処理
    QueryPerformanceCounter(&end);
    double time = static_cast<double>(end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
    printf("time %lf[ms]\n", time);
    //＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊ ＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊＊ 計測終了

    cv::waitKey(0);
}

void debug2()// べた書きバージョン
{
    clock_t start = clock();
    //画像を入力
    cv::Mat frame1 = cv::imread("C:\\Users\\surfa\\Desktop\\originalExe\\image\\dora.jpg");
    cv::Mat frame2 = cv::imread("C:\\Users\\surfa\\Desktop\\originalExe\\image\\HLSColorSpace.png");
    cv::Mat frame = frame1;

    // 二値化処理
    cv::Mat green, mask;
    cv::cvtColor(frame, mask, cv::COLOR_BGR2HSV);
    cv::inRange(mask, cv::Scalar(30, 100, 0), cv::Scalar(90, 255, 255), green);

    cv::Mat labels, stats, centroids;           // 情報の登録先
    int nLab = cv::connectedComponentsWithStats(green, labels, stats, centroids);   // 情報取得
    shelf label_info;
    std::vector<int> areas;

    // ラベリング情報の登録
    int map_index = 0;                          // 解析情報登録用インデックス
    for (int i = 1; i < nLab; ++i)              // すべての情報を参照していく
    {
        int* statsPtr = stats.ptr<int>(i);              // stats用ポインタ
        //double* centerPtr = centroids.ptr<double>(i);   //centroids用ポインタ
        int area = statsPtr[cv::ConnectedComponentsTypes::CC_STAT_AREA];

        if (area > GREEN_FLOOR_SIZE)            // GREEN_FLOOR_SIZEより小さな領域は無視
        {
            dict detail{
                {"left", statsPtr[cv::ConnectedComponentsTypes::CC_STAT_LEFT] },
                {"top", statsPtr[cv::ConnectedComponentsTypes::CC_STAT_TOP] },
                {"height", statsPtr[cv::ConnectedComponentsTypes::CC_STAT_HEIGHT] },
                {"width", statsPtr[cv::ConnectedComponentsTypes::CC_STAT_WIDTH] },
                {"area", area },
                //{"center_x", static_cast<int>(centerPtr[0]) },
                //{"center_y", static_cast<int>(centerPtr[1]) }
            };
            areas.push_back(area);
            label_info.emplace(map_index, detail);
            map_index++;
        }
    }

    // 2個以下の最大値を抽出
    if (areas.size() > 1)
    {
        // 1個目の最大領域
        auto detail_itr = std::max_element(areas.begin(), areas.end());
        int max_index = std::distance(areas.begin(), detail_itr);
        cv::rectangle(frame, cv::Rect((int)label_info.at(max_index)["left"], (int)label_info.at(max_index)["top"],
            (int)label_info.at(max_index)["width"], (int)label_info.at(max_index)["height"]), cv::Scalar(255, 0, 255), 2);
        areas[max_index] = 0;              // 1個目の最大領域の情報を削除
        // 2個目の最大領域
        detail_itr = std::max_element(areas.begin(), areas.end());
        max_index = std::distance(areas.begin(), detail_itr);
        cv::rectangle(frame, cv::Rect((int)label_info.at(max_index)["left"], (int)label_info.at(max_index)["top"],
            (int)label_info.at(max_index)["width"], (int)label_info.at(max_index)["height"]), cv::Scalar(255, 0, 255), 2);
    }
    else if (areas.size() > 0)
    {
        auto detail_itr = std::max_element(areas.begin(), areas.end());
        auto label_info_itr = label_info.begin();
        int max_index = std::distance(areas.begin(), detail_itr);
        cv::rectangle(frame, cv::Rect((int)label_info.at(max_index)["left"], (int)label_info.at(max_index)["top"],
            (int)label_info.at(max_index)["width"], (int)label_info.at(max_index)["height"]), cv::Scalar(255, 0, 255), 2);
    }
    clock_t end = clock();

    const double time = static_cast<double>(end - start) / CLOCKS_PER_SEC * 1000;
    std::cout << time << "[ms]" << std::endl;
    cv::imshow("plane", frame);
    cv::imshow("green", green);
    cv::waitKey(1);
}

int main(void)
{
    debug1();

}
