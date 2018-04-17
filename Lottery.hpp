#pragma once

#include <vector>
#include <fstream>
#include <random>
#include <string>




class Lottery {
	static const size_t MAXGROUPCNT = 10;

	std::vector<std::vector<size_t>> number;	// くじ
	std::vector<size_t> magnif;				// 当選倍率
	
	size_t sumMagnif;
	
	size_t maxGroupID;

	std::mt19937 mt;

	
public:
	Lottery();
	~Lottery();

	// くじのセッター&ゲッター
	bool registerLottery(const wchar_t *fileName);
	const std::vector<std::vector<size_t>>& getLottery() const { return number; };
	
	// 当選倍率のセット&ゲッター
	bool setMagnification(size_t grade, size_t mag);
	const std::vector<size_t>& getMagnification() const { return magnif; };
	

	// 当選番号の取得
	bool getNumber(size_t &win);
	bool getNumber(size_t &win, std::vector<size_t> &groupList);
	
private:
	void clear();

};