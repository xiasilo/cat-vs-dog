#pragma execution_character_set("utf-8")
#include "catVSdog.h"
#include "cocos2d.h"
#include "SimpleAudioEngine.h"
#define database UserDefault::getInstance()

USING_NS_CC;
using namespace CocosDenshion;

void catVSdog::setPhysicsWorld(PhysicsWorld* world) { m_world = world; }

// tag
// player - 1, box in sky - 2, box in ship - 3, ship - 4

Scene* catVSdog::createScene() {
  srand((unsigned)time(NULL));
  auto scene = Scene::createWithPhysics();

  scene->getPhysicsWorld()->setAutoStep(false);

  // Debug ģʽ
  // ����debugģʽ��ҪsetAutoStep(true) ��ע�͵�update������һ��
  //scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);

  scene->getPhysicsWorld()->setGravity(Vec2(0, -1000.0f));
  auto layer = catVSdog::create();
  layer->setPhysicsWorld(scene->getPhysicsWorld());
  scene->addChild(layer);
  return scene;
}

// on "init" you need to initialize your instance
bool catVSdog::init() {

	if (!Layer::init()) {
		return false;
	}
	visibleSize = Director::getInstance()->getVisibleSize();
	origin = Director::getInstance()->getVisibleOrigin();

	// Ԥ����bgm����Ч
	preloadMusic();
	// ��Ӹ���frame
	setFrame();
	setAni();
	// ���ر��������沢���ø���
	setSprite();
	// ��Ӽ�����
	addListener();
	// �������ã��������ѡ�è�����ĵ���
	spriteFall();

	// ��ӵ�����
	schedule(schedule_selector(catVSdog::update), 0.01f, kRepeatForever, 0.1f);

	//add wind
	wind = Sprite::create("/wind/wind_left_weak.png");
	wind->setPosition(visibleSize.width / 2, visibleSize.height / 5 * 4);
	this->addChild(wind, 1);

	//power set to 0
	power = 0;

	isBoneExist = false;
	isFishExist = false;
	isCatAddingPower = false;
	isDogAddingPower = false;
	lastAttack = "cat";

	//Ѫ��
	Sprite* sp0 = Sprite::create("blood_state.png");
	sp0->setPosition(visibleSize.width / 2, visibleSize.height - sp0->getContentSize().height / 2);
	this->addChild(sp0, 0);

	//è��Ѫ��
	Sprite* sp = Sprite::create("blood.jpg");
	percentageCat = 100;
	pCat = ProgressTimer::create(sp);
	pCat->setType(ProgressTimerType::BAR);
	pCat->setBarChangeRate(Point(1, 0));
	pCat->setMidpoint(Point(1, 0.5));
	pCat->setPercentage(100);
	pCat->setPosition(Vec2(visibleSize.width / 2-232, visibleSize.height - sp->getContentSize().height - 28));
	this->addChild(pCat, 1);
	
	//����Ѫ��
	percentageDog = 100;
	pDog = ProgressTimer::create(sp);
	pDog->setType(ProgressTimerType::BAR);
	pDog->setBarChangeRate(Point(1, 0));
	pDog->setMidpoint(Point(0, 0.5));
	pDog->setPercentage(100);
	pDog->setPosition(Vec2(visibleSize.width / 2 + 238, visibleSize.height - sp->getContentSize().height - 28));
	this->addChild(pDog, 1);
	return true;
}

// Ԥ������Ч
void catVSdog::preloadMusic() {
  auto sae = SimpleAudioEngine::getInstance();
  sae->preloadEffect("catMeow.wav");
  sae->preloadEffect("dogBark.wav");
  sae->preloadEffect("tomatoHurt.mp3");
  sae->preloadBackgroundMusic("bgm.mp3");

  sae->playBackgroundMusic("bgm.mp3", true);
}

// ��ӱ����͸��־���
void catVSdog::setFrame() {
	// cat
	auto texture = Director::getInstance()->getTextureCache()->addImage("/cat/cat_normal.png");
	catNormal = SpriteFrame::createWithTexture(texture, CC_RECT_PIXELS_TO_POINTS(Rect(0, 0, 115, 160)));
	texture = Director::getInstance()->getTextureCache()->addImage("/cat/cat_hit.png");
	catHit = SpriteFrame::createWithTexture(texture, CC_RECT_PIXELS_TO_POINTS(Rect(0, 0, 115, 160)));
	texture = Director::getInstance()->getTextureCache()->addImage("/cat/cat_sick.png");
	catSick = SpriteFrame::createWithTexture(texture, CC_RECT_PIXELS_TO_POINTS(Rect(0, 0, 115, 160)));

	// dog
	texture = Director::getInstance()->getTextureCache()->addImage("/dog/dog_normal.png");
	dogNormal = SpriteFrame::createWithTexture(texture, CC_RECT_PIXELS_TO_POINTS(Rect(0, 0, 130, 185)));
	texture = Director::getInstance()->getTextureCache()->addImage("/dog/dog_hit.png");
	dogHit = SpriteFrame::createWithTexture(texture, CC_RECT_PIXELS_TO_POINTS(Rect(0, 0, 130, 185)));
	texture = Director::getInstance()->getTextureCache()->addImage("/dog/dog_sick.png");
	dogSick = SpriteFrame::createWithTexture(texture, CC_RECT_PIXELS_TO_POINTS(Rect(0, 0, 130, 185)));

	// tomato
	texture = Director::getInstance()->getTextureCache()->addImage("/tomato/tomato_normal.png");
	tomatoNormal = SpriteFrame::createWithTexture(texture, CC_RECT_PIXELS_TO_POINTS(Rect(0, 0, 55, 55)));
	texture = Director::getInstance()->getTextureCache()->addImage("/tomato/tomato_hurt1.png");
	tomatoHurt1 = SpriteFrame::createWithTexture(texture, CC_RECT_PIXELS_TO_POINTS(Rect(0, 0, 55, 55)));
	texture = Director::getInstance()->getTextureCache()->addImage("/tomato/tomato_hurt2.png");
	tomatoHurt2 = SpriteFrame::createWithTexture(texture, CC_RECT_PIXELS_TO_POINTS(Rect(0, 0, 55, 55)));

}

void catVSdog::setAni() {
	dogHurted.reserve(3);
	for (int i = 0; i < 3; i++) {
		string filePath = "/dog/dog_hurted" + to_string(i) + ".png";
		dogHurted.pushBack(SpriteFrame::create(filePath, CC_RECT_PIXELS_TO_POINTS(Rect(0, 0, 130, 185))));
	}

	catHurted.reserve(3);
	for (int i = 0; i < 3; i++) {
		string filePath = "/cat/cat_hurted" + to_string(i) + ".png";
		catHurted.pushBack(SpriteFrame::create(filePath, CC_RECT_PIXELS_TO_POINTS(Rect(0, 0, 115, 160))));
	}

	tomatoHurted.reserve(2);
	for (int i = 1; i < 3; i++) {
		string filePath = "/tomato/tomato_hurt" + to_string(i) + ".png";
		tomatoHurted.pushBack(SpriteFrame::create(filePath, CC_RECT_PIXELS_TO_POINTS(Rect(0, 0, 55, 55))));
	}

	tomatoDie.reserve(6);
	for (int i = 1; i < 7; i++) {
		string filePath = "/tomato/die_ani" + to_string(i) + ".png";
		tomatoDie.pushBack(SpriteFrame::create(filePath, CC_RECT_PIXELS_TO_POINTS(Rect(0, 0, 55, 55))));
	}

}

void catVSdog::setSprite() {
	// add backgroud
	visibleSize = Director::getInstance()->getVisibleSize();
	auto bg = Sprite::create("bg1.png");
	//bg->setPosition(visibleSize / 2);
	bg->setPosition(visibleSize / 2);
	bg->setScale(visibleSize.width / bg->getContentSize().width, visibleSize.height / bg->getContentSize().height);
	this->addChild(bg, 0);

	// add ground
	// set tag 1
	auto ground = Sprite::create("ground.png");
	ground->setScale(visibleSize.width / ground->getContentSize().width, 1);
	ground->setPosition(visibleSize.width / 2, ground->getContentSize().height / 2);
	auto groundBody = PhysicsBody::createBox(ground->getContentSize(), PhysicsMaterial(500.0f, 0.0f, 1.0f));
	// ���κ�������ײ�����¼�����
	groundBody->setCategoryBitmask(0xFFFFFFFF);
	groundBody->setCollisionBitmask(0xFFFFFFFF);
	groundBody->setContactTestBitmask(0x00000000);
	groundBody->setDynamic(false);
	groundBody->setTag(1);
	ground->setPhysicsBody(groundBody);
	this->addChild(ground, 1);
}

// ��Ӽ�����
void catVSdog::addListener() {
	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(catVSdog::onTouchBegan, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(catVSdog::onTouchEnded, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);

	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(catVSdog::onConcactBegin, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);
}

// ������ɫ
void catVSdog::spriteFall() {
	// add tomatoes randomly
	// set tag 2
	for (int i = 0; i < 8; i++) {
		auto tomato = Sprite::createWithSpriteFrame(tomatoNormal);
		tomato->setScale(random(0.8, 2.5));
		tomato->setPosition(rand() % (int)(visibleSize.width - 100) + 60, visibleSize.height);
		auto tomatoBody = PhysicsBody::createBox(tomato->getContentSize(), PhysicsMaterial(50.f, 0.2f, 10.0f));
		tomatoBody->setCategoryBitmask(0x00000002); // 0010
		tomatoBody->setCollisionBitmask(0x00000001);
		tomatoBody->setContactTestBitmask(0x00000001);
		tomatoBody->setTag(2);
		tomatoBody->setRotationEnable(false);
		tomato->setPhysicsBody(tomatoBody);
		tomatoes.push_back(tomato);
		tomatoBodies.push_back(tomatoBody);
		this->addChild(tomato, 2);
	}

	// add cat
	// set tag 3
	cat = Sprite::createWithSpriteFrame(catNormal);
	cat->setPosition(visibleSize.width / 4, visibleSize.height / 2);
	auto catBody = PhysicsBody::createBox(cat->getContentSize(), PhysicsMaterial(200.f, 0.0f, 10.0f));
	// è�ͷ��Ѳ�������ײ
	catBody->setCategoryBitmask(0x00000006); // 0110
	catBody->setCollisionBitmask(0x00000006);
	catBody->setContactTestBitmask(0x00000006);
	catBody->setTag(3);
	catBody->setRotationEnable(false);
	cat->setPhysicsBody(catBody);
	this->addChild(cat, 1);

	// add dog
	// set tag 4
	dog = Sprite::createWithSpriteFrame(dogNormal);
	dog->setPosition(3 * visibleSize.width / 4, visibleSize.height / 2);
	auto dogBody = PhysicsBody::createBox(cat->getContentSize(), PhysicsMaterial(200.f, 0.0f, 10.0f));
	dogBody->setCategoryBitmask(0x00000006); // 0110
	dogBody->setCollisionBitmask(0x00000006);
	dogBody->setContactTestBitmask(0x00000001); // 0001
	dogBody->setTag(4);
	dogBody->setRotationEnable(false);
	dog->setPhysicsBody(dogBody);
	this->addChild(dog, 1);
}

bool catVSdog::onTouchBegan(Touch *touch, Event *event) {
	Vec2 pos = touch->getLocation();
	//���㵽��
	if (dog->getBoundingBox().containsPoint(pos)) {
		//������
		isDogAddingPower = true;
	}
	if (cat->getBoundingBox().containsPoint(pos)) {
		isCatAddingPower = true;
	}
	return true;
}

void catVSdog::onTouchEnded(Touch *touch, Event *event) {
	if (isDogAddingPower == true && lastAttack == "cat") {
		bone = Sprite::create("bone.png");
		auto boneBody = PhysicsBody::createBox(bone->getContentSize(), PhysicsMaterial(20.0f, 0.5f, 1.0f));
		boneBody->setCategoryBitmask(0x00000007);
		boneBody->setCollisionBitmask(0x00000001);
		boneBody->setContactTestBitmask(0x00000001);
		boneBody->setVelocity(Vec2(0-10 * power, 10 * power));
		bone->setPhysicsBody(boneBody);
		bone->setPosition(3 * visibleSize.width / 4 - 100, 300);
		this->addChild(bone, 1);
		isBoneExist = true;
		power = 0;
		isDogAddingPower = false;
		lastAttack = "dog";
	}
	else if (isCatAddingPower == true && lastAttack == "dog") {
		fish = Sprite::create("fish.png");
		auto fishBody = PhysicsBody::createBox(fish->getContentSize(), PhysicsMaterial(20.0f, 0.5f, 1.0f));
		fishBody->setCategoryBitmask(0x00000007);
		fishBody->setCollisionBitmask(0x00000001);
		fishBody->setContactTestBitmask(0x00000001);
		fishBody->setTag(5);
		fish->setPhysicsBody(fishBody);
		fish->setPosition(visibleSize.width / 4 + 100, 300);
		fishBody->setVelocity(Vec2(10 * power, 10 * power));
		this->addChild(fish, 1);
		isFishExist = true;
		power = 0;
		isCatAddingPower = false;
		lastAttack = "cat";
	}
}

// ��������������������������֮��ı����룬��������ҷ�����ײ
// Todo
bool catVSdog::onConcactBegin(PhysicsContact & contact) {
	return true;
}

void catVSdog::update(float dt) {
	this->getScene()->getPhysicsWorld()->step(1 / 100.0f);
	//è������
	if (isDogAddingPower == true || isCatAddingPower == true)
		power++;
	//����ͷ�Ƿ��ҵ�è
	if (isBoneExist) {
		auto boneRec = bone->getBoundingBox();
		auto catRec = cat->getBoundingBox();
		//��ͷ�ҵ�è
		if (catRec.containsPoint(bone->getPosition())) {
			bone->removeFromParent();
			auto catHurtedAnimation = Animation::createWithSpriteFrames(catHurted, 0.1f);
			catHurtedAnimation->setRestoreOriginalFrame(true);
			auto catHurtedAnimate = Animate::create(catHurtedAnimation);
			cat->runAction(Repeat::create(catHurtedAnimate, 1));
			SimpleAudioEngine::getInstance()->playEffect("catMeow.wav", false);


			percentageCat -= 10;
			if (percentageCat < 0)
				percentageCat = 0;
			pCat->setPercentage(percentageCat);
			if (percentageCat == 0) {
				//��ʤ��:����߷��̻�,è�Ǳ��л�ɫ����
				auto firework = CCParticleFireworks::create();
				firework->setPosition(3 * visibleSize.width / 4 + 50, visibleSize.height / 2 - 300);
				this->addChild(firework, 1);

				auto smoke = CCParticleSmoke::create();
				smoke->setPosition(visibleSize.width / 4, visibleSize.height / 2 - 300);
				smoke->setColor(Color3B(96, 96, 96));
				this->addChild(smoke, 1);

				GameOver("dog");
			}

			isBoneExist = false;
		}
		//��ͷû���ҵ�è�����ٶ�Ϊ0ʱ�� ��ʧ
		else if (abs(bone->getPhysicsBody()->getVelocity().x - 0.0f) < 0.000001f) {
			bone->removeFromParent();
			isBoneExist = false;
		}
	}
	//������Ƿ��ҵ���
	else if (isFishExist) {
		auto dogRec = dog->getBoundingBox();
		//���ҵ���
		if (dogRec.containsPoint(fish->getPosition())) {
			fish->removeFromParent();
			auto dogHurtedAnimation = Animation::createWithSpriteFrames(dogHurted, 0.1f);
			dogHurtedAnimation->setRestoreOriginalFrame(true);
			auto dogHurtedAnimate = Animate::create(dogHurtedAnimation);
			dog->runAction(Repeat::create(dogHurtedAnimate, 1));
			SimpleAudioEngine::getInstance()->playEffect("dogBark.wav", false);

			percentageDog -= 10;
			if (percentageDog < 0)
				percentageDog = 0;
			pDog->setPercentage(percentageDog);
			if (percentageDog == 0) {
				//èʤ����è��߷��̻������Ǳ��л�ɫ����
				auto firework = CCParticleFireworks::create();
				firework->setPosition(visibleSize.width / 4 - 50, visibleSize.height / 2 - 300);
				this->addChild(firework, 1);

				auto smoke = CCParticleSmoke::create();
				smoke->setPosition(3 * visibleSize.width / 4, visibleSize.height / 2 - 300);
				smoke->setColor(Color3B(96, 96, 96));
				this->addChild(smoke, 1);

				GameOver("cat");
			}

			isFishExist = false;
		}
		//��û���ҵ��������ٶ�Ϊ0ʱ�� ��ʧ
		else if (abs(fish->getPhysicsBody()->getVelocity().x - 0.0f) < 0.000001f ) {
			fish->removeFromParent();
			isFishExist = false;
		}
	}

	//����ͷ�Ƿ��ҵ�����
	if (isBoneExist) {
		for (list<Sprite*>::iterator iter = tomatoes.begin(); iter != tomatoes.end(); iter++) {
			auto tomatoRec = (*iter)->getBoundingBox();
			if (tomatoRec.containsPoint(bone->getPosition())) {
				SimpleAudioEngine::getInstance()->playEffect("tomatoHurt.mp3", false);

				if ((*iter)->isFrameDisplayed(tomatoNormal)) {
					auto tomatoHurtedAnimation = Animation::createWithSpriteFrames(tomatoHurted, 0.1f);
					auto tomatoHurtedAnimate = Animate::create(tomatoHurtedAnimation);
					(*iter)->runAction(Repeat::create(tomatoHurtedAnimate, 1));
				}
				else if ((*iter)->isFrameDisplayed(tomatoHurted.back())) {
					auto tomatoDieAnimation = Animation::createWithSpriteFrames(tomatoDie, 0.1f);
					auto tomatoDieAnimate = Animate::create(tomatoDieAnimation);
					Sequence* seq = Sequence::create(tomatoDieAnimate, CallFunc::create(CC_CALLBACK_0
						(Sprite::removeFromParent, (*iter))), NULL);
					(*iter)->runAction(seq);

					iter = tomatoes.erase(iter);
				}
				bone->removeFromParent();
				isBoneExist = false;
				break;
			}
		}
	}
	else if (isFishExist) {
		//������ҷ���
		for (list<Sprite*>::iterator iter = tomatoes.begin(); iter != tomatoes.end(); iter++) {
			auto tomatoRec = (*iter)->getBoundingBox();
			if (tomatoRec.containsPoint(fish->getPosition())) {
				SimpleAudioEngine::getInstance()->playEffect("tomatoHurt.mp3", false);

				if ((*iter)->isFrameDisplayed(tomatoNormal)) {
					auto tomatoHurtedAnimation = Animation::createWithSpriteFrames(tomatoHurted, 0.1f);
					auto tomatoHurtedAnimate = Animate::create(tomatoHurtedAnimation);
					(*iter)->runAction(Repeat::create(tomatoHurtedAnimate, 1));
				}
				else if ((*iter)->isFrameDisplayed(tomatoHurted.back())) {
					auto tomatoDieAnimation = Animation::createWithSpriteFrames(tomatoDie, 0.1f);
					auto tomatoDieAnimate = Animate::create(tomatoDieAnimation);
					Sequence* seq = Sequence::create(tomatoDieAnimate, CallFunc::create(CC_CALLBACK_0
					(Sprite::removeFromParent, (*iter))), NULL);
					(*iter)->runAction(seq);

					iter = tomatoes.erase(iter);
				}
				fish->removeFromParent();
				isFishExist = false;
				break;
			}
		}
	}
}

void catVSdog::GameOver(string whoWins) {
	unschedule(schedule_selector(catVSdog::update));
	SimpleAudioEngine::getInstance()->stopBackgroundMusic("bgm.mp3");
	//SimpleAudioEngine::getInstance()->playEffect("gameover.mp3", false);

	auto label1 = Label::create();
	if (whoWins == "cat")
		label1 = Label::createWithTTF("CAT WINS!", "fonts/Marker Felt.ttf", 60);
	else 
		label1 = Label::createWithTTF("DOG WINS!", "fonts/Marker Felt.ttf", 60);
	label1->setColor(Color3B(0, 0, 0));
	label1->setPosition(visibleSize.width / 2, visibleSize.height / 2);
	this->addChild(label1);

	auto label2 = Label::createWithTTF("REPLAY", "fonts/Marker Felt.ttf", 40);
	label2->setColor(Color3B(0, 0, 0));
	auto replayBtn = MenuItemLabel::create(label2, CC_CALLBACK_1(catVSdog::replayCallback, this));
	Menu* replay = Menu::create(replayBtn, NULL);
	replay->setPosition(visibleSize.width / 2 - 80, visibleSize.height / 2 - 100);
	this->addChild(replay);

	auto label3 = Label::createWithTTF("EXIT", "fonts/Marker Felt.ttf", 40);
	label3->setColor(Color3B(0, 0, 0));
	auto exitBtn = MenuItemLabel::create(label3, CC_CALLBACK_1(catVSdog::exitCallback, this));
	Menu* exit = Menu::create(exitBtn, NULL);
	exit->setPosition(visibleSize.width / 2 + 90, visibleSize.height / 2 - 100);
	this->addChild(exit);
}

void catVSdog::replayCallback(Ref * pSender) {
	Director::getInstance()->replaceScene(catVSdog::createScene());
}

void catVSdog::exitCallback(Ref * pSender) {
	Director::getInstance()->end();
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	exit(0);
#endif
}

