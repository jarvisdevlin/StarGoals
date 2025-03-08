#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <fstream>
#include <ctime>
#include <string>
#include <Geode/utils/file.hpp>

using namespace geode::prelude;

std::string getDevlin() {
	return Mod::get()->getConfigDir().string() + "/stargoals.devlin";
}

int stars_AllTime() {
	return GameStatsManager::sharedState()->getStat("6");
}

std::string getDate() {
	time_t now = time(0);
	tm* ltm = localtime(&now);
	char buffer[11];
	strftime(buffer, sizeof(buffer), "%Y-%m-%d", ltm);
	return std::string(buffer);
}

void start() {
	int totalStars = stars_AllTime();
	std::ofstream outFile(getDevlin());
	if (outFile.is_open()) {
		outFile << "TOTAL=" << totalStars << "\n";
		outFile << "DAILY=0\n";
		outFile << "LAST_DATE=" << getDate() << "\n";
		outFile.close();
	}
}

void updateStars() {
	int totalStars = stars_AllTime();
	std::ifstream inFile(getDevlin());
	int daily = 0, total = 0;
	std::string line, lastDate;

	if (!inFile.is_open()) {
		start();
		inFile.open(getDevlin());
	}

	if (inFile.is_open()) {
		while (std::getline(inFile, line)) {
			if (line.find("TOTAL") != std::string::npos) {
				total = std::stoi(line.substr(line.find('=') + 1));
			} else if (line.find("DAILY") != std::string::npos) {
				daily = std::stoi(line.substr(line.find('=') + 1));
			} else if (line.find("LAST_DATE") != std::string::npos) {
				lastDate = line.substr(line.find('=') + 1);
			}
		}
		inFile.close();
	}

	std::string currentDate = getDate();

	if (currentDate != lastDate) daily = 0;

	int newStars = totalStars - total;
	daily += newStars;

	std::ofstream outFile(getDevlin());
	if (outFile.is_open()) {
		outFile << "TOTAL=" << totalStars << "\n";
		outFile << "DAILY=" << daily << "\n";
		outFile << "LAST_DATE=" << currentDate << "\n";
		outFile.close();
	}
}

int getStars(const std::string& period) {
	std::ifstream inFile(getDevlin());
	std::string line;
	int stars = 0;

	if (inFile.is_open()) {
		while (std::getline(inFile, line)) {
			if (line.find(period) != std::string::npos) {
				stars = std::stoi(line.substr(line.find('=') + 1));
				break;
			}
		}
		inFile.close();
	}
	return stars;
}

class $modify(MyPauseLayer, PauseLayer) {
	void customSetup() {
		PauseLayer::customSetup();

		auto menu = this->getChildByID("right-button-menu");
        if (!menu) {
            log::error("NAHHHHHHH DATS CRAZY :sob:");
            return;
        }

		try {
			auto stargoalsBtn = CCSprite::createWithSpriteFrameName("GJ_starBtn_001.png");
			CCMenuItemSpriteExtra* button = CCMenuItemSpriteExtra::create(stargoalsBtn, this, menu_selector(MyPauseLayer::onButton));    
			menu->addChild(button);
			menu->updateLayout();
		} catch (const std::exception& e) {
			log::error("i hope your game crashes :skull:");
		}
	}

	void onButton(CCObject* sender) {
		updateStars();
		int totalStars = stars_AllTime();
		int starsToday = getStars("DAILY");
		int goal = 250;

		std::string message = "<cp>---------- Goals ----------</c>\n";
		message += "<cj>Daily Goal:</c> " + std::to_string(starsToday) + " / " + std::to_string(goal) + "\n\n";
		message += "<cp>---------- Stars Earned ----------</c>\n";
		message += "<cj>All Time:</c> " + std::to_string(totalStars) + "\n";
		message += "<cj>Today:</c> " + std::to_string(starsToday) + "\n\n";
		message += "<cy>Daily star count resets after the current day is over.</c>";

		FLAlertLayer::create("StarGoals", message.c_str(), "OK")->show();
	}
};

class $modify(MyPlayLayer, PlayLayer) {
	void levelComplete() {
		PlayLayer::levelComplete();
		updateStars();
	}
};