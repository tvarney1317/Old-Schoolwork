package abSearch;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;

public class ThomasV {
	public static Player getPlayer() { return new TVPlayer(); }
	
	public static class TVPlayer implements Player {
		 public Card playFirstCard(ArrayList<Card> hand, ArrayList<Card> playedCards, Card trump, int tricks1, int tricks2) {
			 int handSize = hand.size();
			 int temp;
			 int trumpNumber;
			 int bestCard = -1; //stores indexes of best cards 
			 int mostWins = Integer.MIN_VALUE;
			 if(handSize == 1) return hand.get(0); //test for trivial case
			 
			 int wins[] = new int[handSize]; //tracks wins for every card in hand
			 ArrayList<Integer> localHand = new ArrayList<Integer>(handSize); //integer version of each card in hand
			 ArrayList<Integer> localPlayedCards = new ArrayList<Integer>(handSize); //integer version of each card in playedCards
			 ArrayList<Integer> nonPlayedCards = 
					 new ArrayList<Integer>(Arrays.asList(0,1,2,4,5,6,8,9,10,12,13,14,16,17,18,20,21,22,24,25,26,28,29,30,32,33,34,36,37,38));
			 ArrayList<Integer> opponentHand;
			 
			 //convert cards to ints and establish the list of cards not played yet
			 trumpNumber = trump.getNumber();
			 
			 for(Card c: hand)
				 localHand.add(c.getNumber()); 
			 
			 for(Card c: playedCards)
				 localPlayedCards.add(c.getNumber());
			 
			 for(int i = 0; i < 39; i++){
				 if(localHand.contains(i) || localPlayedCards.contains(i))
					 nonPlayedCards.remove(nonPlayedCards.indexOf(i));
			 }
		
			 //main testing loop
			 for(int i = 0; i < 1000; i++){
				 Collections.shuffle(nonPlayedCards);
				 opponentHand = new ArrayList<Integer>(nonPlayedCards.subList(0, handSize)); //generate random opponent hand
				 
				 for(int j = 0; j < handSize; j++) {
					 temp = localHand.remove(j);
					 localPlayedCards.add(temp);
					 if(minValue(opponentHand, localHand, localPlayedCards, trumpNumber, tricks1, tricks2, Integer.MIN_VALUE, Integer.MAX_VALUE) > 0) 
						 wins[j]++;
					 //undo the move for the next iteration
					 localHand.add(j, localPlayedCards.remove(localPlayedCards.size() - 1));
				 }
			 }
			 
			 for(int i = 0; i < handSize; i++) {
				 if(wins[i] > mostWins) {
					 bestCard = i;
					 mostWins = wins[i];
				 }
			 }
			 
			 return hand.get(bestCard);
		 }
		 public Card playSecondCard(ArrayList<Card> hand, ArrayList<Card> playedCards, Card trump, int tricks1, int tricks2) {
			 int handSize = hand.size();
			 int temp;
			 int trumpNumber;
			 int bestCard = -1;
			 int mostWins = Integer.MIN_VALUE;
			 if(handSize == 1) return hand.get(0); //test for trivial case
			 
			 int wins[] = new int[handSize]; //tracks wins for every card in hand
			 ArrayList<Integer> localHand = new ArrayList<Integer>(handSize); //integer version of each card in hand
			 ArrayList<Integer> localPlayedCards = new ArrayList<Integer>(handSize); //integer version of each card in playedCards
			 ArrayList<Integer> nonPlayedCards = 
					 new ArrayList<Integer>(Arrays.asList(0,1,2,4,5,6,8,9,10,12,13,14,16,17,18,20,21,22,24,25,26,28,29,30,32,33,34,36,37,38));
			 ArrayList<Integer> opponentHand;
			 
			 //convert cards to ints and establish the list of cards not played yet
			 trumpNumber = trump.getNumber();
			 
			 for(Card c: hand)
				 localHand.add(c.getNumber()); 
			 
			 for(Card c: playedCards)
				 localPlayedCards.add(c.getNumber());
			 
			 for(int i = 0; i < 39; i++){
				 if(localHand.contains(i) || localPlayedCards.contains(i) || i == trumpNumber)
					 nonPlayedCards.remove(nonPlayedCards.indexOf(i));
			 }
		
			 //main testing loop
			 for(int i = 0; i < 1000; i++){
				 Collections.shuffle(nonPlayedCards);
				 opponentHand = new ArrayList<Integer>(nonPlayedCards.subList(0, handSize - 1)); //generate random opponent hand
				 
				 for(int j = 0; j < handSize; j++) {
					 temp = localHand.remove(j);
					 localPlayedCards.add(temp);
					 //evaluate second card played because end of trick
					 if(playedCards.get(playedCards.size() - 1).greater(hand.get(j), trump)) {
						 tricks1++;
						 if(maxValue(opponentHand, localHand, localPlayedCards, trumpNumber, tricks1, tricks2, Integer.MIN_VALUE, Integer.MAX_VALUE) < 0)
							 wins[j]++;
						 tricks1--;
					 }
					 else {
						 tricks2++;
						 if(maxValue(opponentHand, localHand, localPlayedCards, trumpNumber, tricks1, tricks2, Integer.MIN_VALUE, Integer.MAX_VALUE) < 0)
							 wins[j]++;
						 tricks2--;
					 }
					 //undo the move for the next iteration
					 localHand.add(j, localPlayedCards.remove(localPlayedCards.size() - 1));
				 }
			 }
			 
			 //pick the card that wins most often from the trump suit or the opponent's suit, else throw any random card
			 for(int i = 0; i < handSize; i++) {
				 if(wins[i] > mostWins && (hand.get(i).suit == trump.suit  || hand.get(i).suit == playedCards.get(playedCards.size() - 1).suit)) {
					 bestCard = i;
					 mostWins = wins[i];
				 }
			 }
			 
			 if(bestCard == -1) { //if we have no cards of trump suit or opponent's suit
				 for(int i = 0; i < handSize; i++) {
					 if(wins[i] > mostWins) {
						 bestCard = i;
						 mostWins = wins[i];
					 }
				 }  
			 }
			 return hand.get(bestCard);
		 }
		 
		 //these return 1 or -1, respectively player 1 wins or player 2 wins
		 private int minValue(ArrayList<Integer> hand, ArrayList<Integer> opponent, ArrayList<Integer> playedCards, int trump, 
				 int tricks1, int tricks2, int alpha, int beta) {
			 if(tricks1 > 3) return 1;
			 else if(tricks2 > 3) return -1;
			 
			 int v = Integer.MAX_VALUE;
			 int temp;
			 
			 for(int i = 0; i < hand.size(); i++) {
				 if(greater(hand.get(i), playedCards.get(playedCards.size() - 1), trump)) {
					 tricks2++;
					 temp = hand.remove(i);
					 playedCards.add(temp);
					 v = min(v, maxValue(opponent,hand,playedCards,trump,tricks1,tricks2,alpha,beta));
					 if(v <= alpha) {
						 tricks2--;
						 hand.add(i, playedCards.remove(playedCards.size() - 1));
						 return v;
					 }
					 beta = min(beta, v);
					 tricks2--;
				 }
				 else {
					 tricks1++;
				 	 temp = hand.remove(i);
				 	 playedCards.add(temp);
				 	 v = min(v, maxValue(opponent,hand,playedCards,trump,tricks1,tricks2,alpha,beta));
				 	 if(v <= alpha) {
				 		 tricks1--;
				 		 hand.add(i, playedCards.remove(playedCards.size() - 1));
				 		 return v;
				 	 }
				 	 beta = min(beta, v);
				 	 tricks1--;
				 }
				 hand.add(i, playedCards.remove(playedCards.size() - 1));
			 }
			 return v;
		 }
		 private int maxValue(ArrayList<Integer> hand, ArrayList<Integer> opponent, ArrayList<Integer> playedCards, int trump,
				 int tricks1, int tricks2, int alpha, int beta) {
			 if(tricks1 > 3) return 1;
			 else if(tricks2 > 3) return -1;
			 
			 int v = Integer.MIN_VALUE;
			 int temp;
			 
			 for(int i = 0; i < hand.size(); i++) {
				 temp = hand.remove(i);
				 playedCards.add(temp);
				 v = max(v, minValue(opponent,hand,playedCards,trump,tricks1,tricks2,alpha,beta));
				 if(v >= beta) {
					 hand.add(i, playedCards.remove(playedCards.size() - 1));
					 return v;
				 }
				 alpha = max(alpha, v);
				 hand.add(i, playedCards.remove(playedCards.size() - 1));
			 }
			 return v;
		 }
		 private boolean greater(int a, int b, int trump) {
			 int trumpSuit = trump % 4;
			 int aSuit = a % 4;
			 int bSuit = b % 4;
			 int aValue = (a - aSuit) / 4;
			 int bValue = (b - bSuit) / 4;
			 
			 if(aSuit == bSuit)
				 return aValue > bValue;
			 return aSuit == trumpSuit;
		 }
		 private int max(int a, int b) {
			 return (a > b) ? a : b;
		 }
		 private int min(int a, int b) {
			 return (a < b) ? a : b;
		 }
	}
}
