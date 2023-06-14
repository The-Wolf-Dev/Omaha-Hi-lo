#include <iostream>

#include <Hand.h>
#include <Combinations.h>
#include <Checker.h>
#include <Tools.h>

Hand::Hand(std::string handSetup) 
	: m_maxHighHandRank{ HighHandRank::None }, m_hasLowHand{ false } {
	for (size_t i{ 0U }, end{ handSetup.length() }; i < end; ++i)
	{
		if (i % 3 == 0) {
			m_cards.emplace_back(parseRank(handSetup[i]), parseSuit(handSetup[i + 1U]));
		}
	}
	std::sort(m_cards.begin(), m_cards.end(), std::greater<Card>());
}

HighHandRank Hand::getHighHandRank() const {
	return m_maxHighHandRank;
}

std::vector<Card> Hand::getHighHandCards() const {
	return m_maxHighHandCards;
}

bool Hand::hasLowHand() const {
	return m_hasLowHand;
}

std::vector<Card> Hand::getLowHandCards() const {
	return m_lowHandCards;
}

void Hand::findHiLoHand(const Board& board) {
	const auto boardCards = board.getCards();
	HighHandRank highHandRank = HighHandRank::None;
	std::vector<Card> cards, twoCards, threeCards;

	Combinations<Card> handComb(m_cards, PAIR_FROM_HAND), boardComb(boardCards, THREE_FROM_BOARD);

	cards.reserve(HILO_HAND_SIZE);
	while (handComb.generateCombination(twoCards))
	{
		while (boardComb.generateCombination(threeCards)) {
			cards.insert(cards.end(), twoCards.begin(), twoCards.end());
			cards.insert(cards.end(), threeCards.begin(), threeCards.end());

			highHandRank = checkHighHandCards(cards);
			checkHighHandRank(highHandRank, cards);
			checkLowHandCards(cards);

			cards.clear();
		}
	}
}

void Hand::checkLowHandCards(std::vector<Card>& cards) {
	auto uniqueCardsCount = std::unique(cards.begin(), cards.end()) - cards.begin();
	if (uniqueCardsCount < HILO_HAND_SIZE) return;

	auto aceCard = findHighAce(cards);
	if (aceCard != cards.end()) makeLowAce(aceCard);
	std::sort(cards.begin(), cards.end());

	auto notSuitableCard = findNotSuitableCard(cards);
	if (notSuitableCard != cards.cend()) return;

	if (cards < m_lowHandCards || m_lowHandCards.empty()) {
		m_lowHandCards.assign(cards.begin(), cards.end());
		std::reverse(m_lowHandCards.begin(), m_lowHandCards.end());
		m_hasLowHand = true;
	}
}

HighHandRank Hand::checkHighHandCards(std::vector<Card>& highHandCards) {
	std::sort(highHandCards.begin(), highHandCards.end(), std::greater<Card>());
	Checker checker;

	for (const auto& c : highHandCards) {
		checker.update(c);
	}

	if (checker.isStraightFlush()) {
		handleExceptionalStraightFlush(highHandCards);
		return HighHandRank::StraightFlush;
	} 
	else if (checker.is4OfAKind()) {
		return HighHandRank::FourOfAKind;
	}
	else if (checker.isFullHouse()) {
		return HighHandRank::FullHouse;
	}
	else if (checker.isFlush()) {
		return HighHandRank::Flush;
	}
	else if (checker.isStraight()) {
		handleExceptionalStraightFlush(highHandCards);
		return HighHandRank::Straight;
	}
	else if (checker.is3OfAKind()) {
		return HighHandRank::ThreeOfAKind;
	}
	else if (checker.isTwoPair()) {
		return HighHandRank::TwoPair;
	}
	else if (checker.isOnePair()) {
		return HighHandRank::OnePair;
	}
	else {
		return HighHandRank::HighCard;
	}
}

void Hand::checkHighHandRank(const HighHandRank& highHandRank, const std::vector<Card>& highHandCards) {
	if (m_maxHighHandRank < highHandRank) {
		m_maxHighHandRank = highHandRank;
		m_maxHighHandCards.assign(highHandCards.begin(), highHandCards.end());
	}
	else if (m_maxHighHandRank == highHandRank && m_maxHighHandCards < highHandCards) {
		m_maxHighHandCards.assign(highHandCards.begin(), highHandCards.end());
	}
}