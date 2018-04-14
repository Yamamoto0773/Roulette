#include "Lottery.hpp"

Lottery::Lottery() {
	// 初期値セット
	magnif.resize(MAXGROUPCNT);
	name.resize(MAXGROUPCNT);

	clear();

	std::random_device rnd;
	mt.seed(rnd());

}

Lottery::~Lottery() {
}

// くじのセット
bool Lottery::registerLottery(const wchar_t * fileName) {
	clear();

	if (fileName == nullptr) {
		return false;
	}

	using namespace std;

	wifstream wifs(fileName);
	if (!wifs) {
		return false;
	}

	wstring tmp;
	wstring param1, param2;

	size_t group;
	size_t i;
	while (getline(wifs, tmp)) {
		i = 0;
		param1.clear();
		param2.clear();
		group = 0;

		while (tmp[i] == ' ' || tmp[i] == ',' || tmp[i] == '\t') i++;	// 区切り文字は飛ばす


		while (tmp[i] >= '0' && tmp[i] <= '9') {
			group *= 10;
			group += tmp[i]-'0';
			i++;
		}

		while (tmp[i] == ' ' || tmp[i] == ',' || tmp[i] == '\t') i++;	// 区切り文字は飛ばす



		while (tmp[i] >= '0' && tmp[i] <= '9') {	// くじ取得
			param1.push_back(tmp[i]);
			i++;
		}

		while (tmp[i] == ' ') i++;	// 区切り文字は飛ばす

		if (tmp[i] == '-') {
			// 範囲指定コマンドの場合
			i++;

			while (tmp[i] == ' ') i++;	// 区切り文字は飛ばす

			while (tmp[i] >= '0' && tmp[i] <= '9') {	// 番号取得
				param2.push_back(tmp[i]);
				i++;
			}

			if (stoi(param2)-stoi(param1) < 0) {
				return false; // 2値の大小関係が逆
			}

		}

		// 以下，正しいフォーマットの場合の処理

		if (param2.size() == 0) {
			name[group-1].push_back(stoi(param1));
		}
		else {
			// 範囲指定の場合
			for (size_t i=stoi(param1); i <= stoi(param2); i++) {
				name[group-1].push_back(i);
			}

		}


		if (magnif[group-1] == 0) {
			magnif[group-1] = 1;	// 登録されたグループの倍率を1にする
		}

		if (group > maxGroupID) {
			maxGroupID = group;
		}


		tmp.clear();

	}


	sumMagnif = 0;
	for (size_t i=0; i<magnif.size(); i++) {
		sumMagnif += magnif[i];
	}

	return true;
}

bool Lottery::setMagnification(size_t grade, size_t mag) {
	if (grade < 1 || grade > maxGroupID) {
		return false;
	}

	magnif[grade-1] = mag;

	return true;
}


// 当選番号の取得
bool Lottery::getNumber(size_t &win) {
	std::vector<size_t> v;

	for (size_t i=1; i<=maxGroupID; i++) {
		v.push_back(i);
	}

	return getNumber(win, v);
}


// 当選番号の取得
bool Lottery::getNumber(size_t &win, std::vector<size_t> &groupList) {


	/////////////////////////////////////////////////////
	// 以下，エラー処理

	if (sumMagnif == 0 || maxGroupID == 0) {
		return false;
	}

	// くじが存在するかチェック
	size_t sum = 0;
	for (size_t g : groupList) {
		if (g < 1 || g > maxGroupID) {
			return false;
		}
		sum += name[g-1].size();
	}
	if (sum == 0) {
		return false;
	}

	/////////////////////////////////////////////////////


	size_t selectGroup = 0;


	// グループの決定処理

	std::vector<size_t> threshold;

	// 境界値作成
	for (size_t g : groupList) {
		if (threshold.size() == 0)
			threshold.push_back(name[g-1].size()*magnif[g-1]);
		else
			threshold.push_back(threshold.back() + name[g-1].size()*magnif[g-1]);
	}

	// 乱数を発生させ，その乱数がどの領域にいるのか調べる
	unsigned int mtrand = mt()%threshold.back();

	size_t cnt = 0;
	for (size_t t : threshold) {
		if (mtrand < t) {
			break;
		}
		cnt++;
	}

	selectGroup = groupList[cnt];


	// グループが決まったら，くじを決める
	size_t pos = mt()%name[selectGroup-1].size();

	win = name[selectGroup-1][pos];	// コピー


	// 当たったくじの削除処理
	size_t tmp = name[selectGroup-1].back();
	name[selectGroup-1].back() = name[selectGroup-1][pos];
	name[selectGroup-1][pos] = tmp;
	name[selectGroup-1].pop_back();




	return true;
}


void Lottery::clear() {

	for (size_t i=0; i<name.size(); i++) {
		name[i].clear();
	}

	for (size_t i=0; i<magnif.size(); i++) {
		magnif[i] = 0;
	}

	sumMagnif = 0;
	maxGroupID = 0;

}
