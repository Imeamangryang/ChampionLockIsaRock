#include "TFTShop.h"
#include <algorithm>
#include <random>
#include <stdexcept>

// ============================================================
//  TFT Shop Widget - TFTShop.cpp
//  Set 16 기준 리롤 확률 + 골드 시스템
// ============================================================

// 티어별 placeholder 챔피언 이름 (블루프린트용 더미 데이터)
// 실제 프로젝트에서는 DataTable이나 외부 JSON으로 교체하세요
static const std::array<std::vector<std::string>, 5> CHAMPION_POOL = {{
    /* 1cost */ { "Caitlyn", "Darius", "Garen", "Jinx", "Malphite" },
    /* 2cost */ { "Ahri", "Ezreal", "Fiora", "Graves", "Lux" },
    /* 3cost */ { "Camille", "Jayce", "Orianna", "Syndra", "Vi" },
    /* 4cost */ { "Ekko", "Jinx (4)", "Silco", "Viktor", "Warwick" },
    /* 5cost */ { "Augmented Viktor", "Broken Machine", "Enforcer Prime", "Jhin", "Zed" },
}};

// ────────────────────────────────────────────────────────────
//  생성자
// ────────────────────────────────────────────────────────────
TFTShop::TFTShop(int startGold, int startLevel)
    : gold_(std::clamp(startGold, 0, MAX_GOLD))
    , level_(std::clamp(startLevel, 1, MAX_LEVEL))
    , currentExp_(0)
{
    refreshShop();
}

// ────────────────────────────────────────────────────────────
//  골드 관리
// ────────────────────────────────────────────────────────────
bool TFTShop::addGold(int amount) {
    if (amount <= 0) return false;
    gold_ = std::min(gold_ + amount, MAX_GOLD);
    if (onStateChanged) onStateChanged();
    return true;
}

bool TFTShop::spendGold(int amount) {
    if (amount <= 0 || gold_ < amount) return false;
    gold_ -= amount;
    if (onStateChanged) onStateChanged();
    return true;
}

// ────────────────────────────────────────────────────────────
//  레벨 / EXP 관리
// ────────────────────────────────────────────────────────────
int TFTShop::getExpToNext() const {
    if (isMaxLevel()) return 0;
    // 현재 레벨에서 다음 레벨까지 필요한 EXP
    return LEVEL_UP_EXP[level_ - 1] - currentExp_;
}

void TFTShop::addExp(int amount) {
    if (amount <= 0 || isMaxLevel()) return;
    currentExp_ += amount;
    tryLevelUp();
    if (onStateChanged) onStateChanged();
}

void TFTShop::tryLevelUp() {
    // 최대 레벨에 도달할 때까지 반복 체크
    while (!isMaxLevel()) {
        int needed = LEVEL_UP_EXP[level_ - 1];
        if (currentExp_ < needed) break;
        currentExp_ -= needed;
        level_++;
    }
    if (isMaxLevel()) currentExp_ = 0;
}

// ────────────────────────────────────────────────────────────
//  리롤 (골드 2 차감 → 샵 갱신)
// ────────────────────────────────────────────────────────────
bool TFTShop::reroll() {
    if (!spendGold(REROLL_COST)) return false;   // 골드 부족
    refreshShop();
    if (onStateChanged) onStateChanged();
    return true;
}

// ────────────────────────────────────────────────────────────
//  EXP 구매 (골드 4 차감 → EXP +4)
//  레벨업 가능하면 자동으로 레벨 올라감
// ────────────────────────────────────────────────────────────
bool TFTShop::buyXP() {
    if (isMaxLevel())           return false;   // 이미 최대 레벨
    if (!spendGold(BUY_XP_COST)) return false;  // 골드 부족
    currentExp_ += BUY_XP_AMOUNT;
    tryLevelUp();
    if (onStateChanged) onStateChanged();
    return true;
}

// ────────────────────────────────────────────────────────────
//  현재 레벨의 티어별 확률 반환
// ────────────────────────────────────────────────────────────
std::array<float, 5> TFTShop::getRollOdds() const {
    int idx = std::clamp(level_ - 1, 0, MAX_LEVEL - 1);
    return ROLL_ODDS[idx];
}

// ────────────────────────────────────────────────────────────
//  내부: 샵 슬롯 갱신
// ────────────────────────────────────────────────────────────
void TFTShop::refreshShop() {
    for (auto& slot : shop_) {
        Tier t = rollTier();
        slot = makeRandomChampion(t);
    }
}

// ────────────────────────────────────────────────────────────
//  내부: 가중치 기반 랜덤 티어 결정
// ────────────────────────────────────────────────────────────
Tier TFTShop::rollTier() const {
    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> dist(0.f, 1.f);
    float roll = dist(rng);

    auto odds = getRollOdds();
    float cumulative = 0.f;
    for (int i = 0; i < 5; ++i) {
        cumulative += odds[i];
        if (roll < cumulative) return static_cast<Tier>(i);
    }
    return Tier::ONE; // fallback
}

// ────────────────────────────────────────────────────────────
//  내부: 티어에 맞는 랜덤 챔피언 슬롯 생성
// ────────────────────────────────────────────────────────────
ChampionSlot TFTShop::makeRandomChampion(Tier tier) const {
    static std::mt19937 rng{ std::random_device{}() };
    int tierIdx = static_cast<int>(tier);

    const auto& pool = CHAMPION_POOL[tierIdx];
    std::uniform_int_distribution<int> pick(0, static_cast<int>(pool.size()) - 1);

    ChampionSlot slot;
    slot.name    = pool[pick(rng)];
    slot.tier    = tier;
    slot.cost    = tierIdx + 1;   // 티어 == 코스트
    slot.isEmpty = false;
    return slot;
}
