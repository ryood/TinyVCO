/*
 * ATtinyVCO.c
 *
 * Created: 2015/11/25 14:07:46
 *  Author: gizmo
 *
 * PB1: PWM out
 * PB2: check pin
 *
 * 2015.11.27 PWM�o�͂�6bit���x�ɕύX
 *
 */ 

#define F_CPU	9600000ul

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdint.h>

#define SAMPLE_CLOCK	(8000.0f)
#define POW_2_16		(65536ul)

#define FREQUENCY_MAX	(2000.0f)

// Saw dawn wave up table
const PROGMEM uint8_t sawUpTable[] = {
	0,	1,	2,	3,	4,	5,	6,	7,	8,	9,	10,	11,	12,	13,	14,	15,
	16,	17,	18,	19,	20,	21,	22,	23,	24,	25,	26,	27,	28,	29,	30,	31,
	32,	33,	34,	35,	36,	37,	38,	39,	40,	41,	42,	43,	44,	45,	46,	47,
	48,	49,	50,	51,	52,	53,	54,	55,	56,	57,	58,	59,	60,	61,	62,	63
};

volatile uint16_t phaseAccumlator;
volatile uint16_t tuningWord;
volatile uint8_t amp;

//=============================================================================
// �g�`����
//
// ----------------------------------------------------------------------------
// setDDSParameter()
// parameter: frequency: ����������g��
//
void setDDSParameter(float frequency)
{
	tuningWord = (int16_t)(frequency * POW_2_16 / SAMPLE_CLOCK);
}

// ----------------------------------------------------------------------------
// generateWave()
// return: �o�͒l(0..255)
//
uint8_t generateSawWave()
{
	uint8_t index;
	
	phaseAccumlator += tuningWord;
	
	// �E�փV�t�g: 16bit -> 6bit(64��)
	index = phaseAccumlator >> 10;
	return pgm_read_byte(&sawUpTable[index]);
}

//=============================================================================
// �g�`����
//
// ----------------------------------------------------------------------------
// setPWMDuty()
// parameter: value: �ݒ肷��Duty(0 .. OCR0A)
//
void setPWMDuty(uint8_t value)
{
	// PWM�̃f���[�e�B�[���ݒ�
	OCR0B = value;
}

//=============================================================================
// ADC
//
// ----------------------------------------------------------------------------
// getCV()
// return: CV�l(0..4095)
//
uint16_t getCV() {
	// ADC_CV�̒l���擾
	 return 901;	// 440Hz
	//return 4095;	// 2000Hz
}

// ----------------------------------------------------------------------------
// getGate()
// return: Gate�l(0..255)
//
uint8_t getGate() {
	// ADC_GATE�̒l���擾
	return 255;
}

//=============================================================================
// ���b�`������
//
//ISR()
//	generateSawWave()���Ăяo��
//	amp�l����Z
//	PWM DAC�ɏo�͒l��ݒ�

//=============================================================================
// ���C���E���[�`��
//
int main()
{
	uint16_t cv;
	uint8_t v;
	
	//-------------------------------------------------------------------------
	// PORT�ݒ�
	//-------------------------------------------------------------------------
	// DDRB = 0;
	DDRB |= (1 << DDB1);	// PB1(OC0B): PWM out
	// Debug�p
	//
	DDRB |= (1 << DDB2);	// PB2: output
	
	//-------------------------------------------------------------------------
	// PWM�ݒ�
	//-------------------------------------------------------------------------
	// TCCR0A = 0;
	// TCCR0B = 0;
	//-------------------------------------------------------------------------
	// �g�`�������[�h: WGM0: 1:1:1
	// ����PWM(���[�h7)
	TCCR0A |= (1 << WGM01) | (1 << WGM00);
	TCCR0B |= (1 << WGM02);
	//-------------------------------------------------------------------------
	// �R���y�A�E�A�E�g�v�b�gB: COM0B: 1:0
	// �R���y�A�E�}�b�`��OC0B�N���A�ATOP��OC0B�Z�b�g
	TCCR0A |= (1 << COM0B1) | (0 << COM0B0);
	//-------------------------------------------------------------------------
	// �N���b�N�ݒ�: CS0: 0:0:1
	// �����Ȃ�
	TCCR0B |= (0 << CS02) | (0 << CS01) | (1 << CS00);
	
	// TCCR0A = 0b00100011;
	// TCCR0B = 0B00001001;
	
	//-------------------------------------------------------------------------
	// ����\ 6bit(0 .. 63) 
	OCR0A = 64;
	
	for (;;) {
		// Debug�p: PB2
		PORTB |= (1 << PORTB2);
		
		cv = getCV();
		setDDSParameter(cv * FREQUENCY_MAX / 4096.0f);
		amp = getGate();
		v = generateSawWave();
		setPWMDuty(v);
		
		// Debug�p: PB2
		PORTB &= ~(1 << PORTB2);
		
		_delay_us(125);	// 8,000Hz		
	}
}
// EOF