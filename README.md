# Smart Parking System

This project is an automatic smart parking system designed to manage vehicle entry and exit efficiently. It uses IR sensors to detect cars, an LCD display to show the parking count, a servo motor to open and close the gate, and LEDs to indicate the status of the parking lot. The system helps reduce manual effort and makes parking management more organized and smart.

## Project Overview

- Detects vehicles entering and leaving the parking area
- Displays available and occupied parking slots on an LCD
- Opens and closes the gate automatically using a servo motor
- Shows parking status using green and red LEDs
- Provides a simple and practical embedded systems solution

## Project Images

![Smart Parking System Image 1](Image/WhatsApp%20Image%202026-07-25%20at%202.41.28%20PM.jpeg)

![Smart Parking System Image 2](Image/WhatsApp%20Image%202026-07-25%20at%202.41.32%20PM.jpeg)

## Video Demonstration

A demo video for the project can be added here. Please place your video file in the Video folder and update this section with the correct file name.

<video controls width="100%" poster="Image/WhatsApp%20Image%202026-07-25%20at%202.41.32%20PM.jpeg">
  <source src="Video/your-video.mp4" type="video/mp4">
  Your browser does not support the video tag.
</video>

## Source Code

The complete source code for this project is available in [Code/code.c](Code/code.c).

```c
int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM1_Init();

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    Servo_Close();

    LCD_Init();
    Display_Count();

    while (1)
    {
        Check_Entry();
        Check_Exit();
    }
}
```

## Conclusion

This smart parking system is a practical embedded project that combines sensors, motors, display modules, and control logic to create an efficient parking solution.
