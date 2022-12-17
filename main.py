import random


iterations = 1000000
faces = 13
suits = 4

successes = 0

for i in range(iterations):
	deck = list(range(faces*suits))
	random.shuffle(deck)
	
	hand = []
	discard_count = 0

	while len(deck):
		while len(hand) < 4:
			hand.append(deck.pop())
			if not len(deck):
				break

		if not len(deck) and len(hand) < 4:
				break

		if hand[-1]%suits == hand[-4]%suits:
			hand.pop(-3)
			hand.pop(-2)
			discard_count += 2
		elif hand[-1]%faces == hand[-4]%faces:
			for j in range(4):
				hand.pop()
			discard_count += 4
		else:
			if not len(deck):
				break
			hand.append(deck.pop())


	if discard_count == suits*faces:
		successes += 1


print("%d/%d games won, %2.2f%% success rate" % (successes, iterations, 100.0*successes/iterations))








