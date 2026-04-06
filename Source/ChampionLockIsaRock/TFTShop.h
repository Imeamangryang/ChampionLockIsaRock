#pragma once
#include <array>
#include <vector>
#include <string>
#include <functional>

// ============================================================
//  TFT Shop Widget - TFTShop.h
//  Set 16 기준 리롤 확률 + 골드 시스템
// ============================================================

// 챔피언 티어 (1~5cost)
enum class Tier : int {
    ONE   = 0,
    TWO   = 1,
    THREE = 2,
    FOUR  = 3,
    FIVE  = 4
};

// 레벨별 리롤 확률 (레벨 1~10, 티어 1~5)
// Set 16 Lore & Legends 기준
// [level-1][tier] = 확률 (0.0 ~ 1.0)
static constexpr std::array<std::array<float, 5>, 10> ROLL_ODDS = {{
    // Tier:  1      2      3      4      5
    /* Lv1 */ { 1.00f, 0.00f, 0.00f, 0.00f, 0.00f },
    /* Lv2 */ { 1.00f, 0.00f, 0.00f, 0.00f, 0.00f },
    /* Lv3 */ { 0.70f, 0.25f, 0.05f, 0.00f, 0.00f },
    /* Lv4 */ { 0.55f, 0.30f, 0.15f, 0.00f, 0.00f },
    /* Lv5 */ { 0.35f, 0.35f, 0.25f, 0.05f, 0.00f },
    /* Lv6 */ { 0.25f, 0.35f, 0.30f, 0.10f, 0.00f },
    /* Lv7 */ { 0.19f, 0.30f, 0.35f, 0.15f, 0.01f },
    /* Lv8 */ { 0.15f, 0.20f, 0.35f, 0.24f, 0.06f },
    /* Lv9 */ { 0.10f, 0.15f, 0.33f, 0.30f, 0.12f },
    /* Lv10*/ { 0.05f, 0.10f, 0.20f, 0.40f, 0.25f },
}};

// EXP 레벨업 임계값 (레벨 1~9 → 레벨업에 필요한 누적 EXP)
// [현재레벨-1] = 해당 레벨업까지 필요한 총 EXP
static constexpr std::array<int, 9> LEVEL_UP_EXP = {
    0,   // 1→2
    2,   // 2→3
    6,   // 3→4
    10,  // 4→5
    20,  // 5→6
    36,  // 6→7
    56,  // 7→8
    80,  // 8→9
    100  // 9→10
};

// 골드 관련 상수
static constexpr int REROLL_COST   = 2;   // 리롤 비용
static constexpr int BUY_XP_COST   = 4;   // EXP 구매 비용
static constexpr int BUY_XP_AMOUNT = 4;   // EXP 구매시 획득량
static constexpr int MAX_GOLD      = 99;
static constexpr int MAX_LEVEL     = 10;
static constexpr int SHOP_SIZE     = 5;

// 샵에 표시되는 챔피언 슬롯
struct ChampionSlot {
    std::string name;
    Tier        tier;
    int         cost;
    bool        isEmpty;
};

// ============================================================
//  TFTShop 클래스
// ============================================================
class TFTShop {
public:
    // 생성자: 초기 골드, 레벨 설정
    explicit TFTShop(int startGold = 3, int startLevel = 1);

    // ── 골드 관리 ──────────────────────────────────────────
    int  getGold()  const { return gold_; }
    bool addGold(int amount);          // 골드 추가 (최대 99)
    bool spendGold(int amount);        // 골드 차감 (부족시 false)

    // ── 레벨 / EXP 관리 ────────────────────────────────────
    int  getLevel()      const { return level_; }
    int  getCurrentExp() const { return currentExp_; }
    int  getExpToNext()  const;        // 다음 레벨까지 필요 EXP
    bool isMaxLevel()    const { return level_ >= MAX_LEVEL; }

    // EXP 직접 추가 (골드 차감 없이 — 내부/테스트용)
    void addExp(int amount);

    // ── 리롤 / EXP 구매 ────────────────────────────────────
    // 리롤: 골드 2 차감 → 샵 갱신 → true 반환 (골드 부족시 false)
    bool reroll();

    // EXP 구매: 골드 4 차감 → EXP +4 (레벨업 가능시 자동 처리)
    bool buyXP();

    // ── 샵 슬롯 접근 ───────────────────────────────────────
    const std::array<ChampionSlot, SHOP_SIZE>& getShop() const { return shop_; }

    // 현재 레벨의 티어별 확률 반환
    std::array<float, 5> getRollOdds() const;

    // 콜백: 상태 변경시 호출 (UI 갱신용)
    std::function<void()> onStateChanged;

private:
    int gold_;
    int level_;
    int currentExp_;   // 현재 누적 EXP (레벨 내부)

    std::array<ChampionSlot, SHOP_SIZE> shop_;

    // 샵 슬롯을 현재 레벨 확률 기반으로 채움
    void refreshShop();

    // 내부 레벨업 처리
    void tryLevelUp();

    // 가중치 기반 랜덤 티어 선택
    Tier rollTier() const;

    // 티어에 맞는 더미 챔피언 이름 반환 (블루프린트용 placeholder)
    ChampionSlot makeRandomChampion(Tier tier) const;
};
