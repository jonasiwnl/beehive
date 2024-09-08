import cv2


def main():
    cap = cv2.VideoCapture("rtsp://0.0.0.0:8554/stream")

    while cap.isOpened():
        _, frame = cap.read()
        cv2.imshow('frame', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    cap.release()
    cv2.destroyAllWindows()


if __name__ == '__main__':
    main()
