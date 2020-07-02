 	#megan.K. developed
	def online_playPause(self):


		print('Loading main module---1')
		self.ip = "192.168.7.16"
		self.port= 50660
		self.myCam 	 = src.tofQT5.ToFSystem(self,self.ip,self.port)
		print('Loading main module Done')
		serverConnectionStatus = self.myCam.startup.start()
		print('Server Start Status 1:', serverConnectionStatus)
		self.parent.QLabelStartUp.setText('Loading main module done'+'\n'+'Server Start Status 2:'+str( serverConnectionStatus))
		if not(serverConnectionStatus):
			quit()

		print('Loading main module---2')
		self.ip2 = "192.168.7.17"
		self.port2= 4444
		self.myCam2  = src.tofQT5.ToFSystem(self,self.ip2,self.port2)
		print('Loading main module Done---2')
		serverConnectionStatus2 = self.myCam2.startup.start()
		print('Server Start Status 2:', serverConnectionStatus2)
		
		self.parent.QLabelStartUp.setText('Loading main module done'+'\n'+'Server Start Status 2:'+str( serverConnectionStatus2))
		if not(serverConnectionStatus2):
			quit()

		
		self.myCam.configuration.start()
		self.myCam2.configuration.start()

		self.myCam.controllerView.updateViewMode(ViewMode.amplitudeGrayscale) # grayscale or amplitudeGrayscale
		self.myCam.calibcorrect.updatePiDelay(1)
		self.myCam.controllerView.setMinAmplitude(5)
		

		self.myCam2.controllerView.updateViewMode(ViewMode.amplitudeGrayscale) # grayscale or amplitudeGrayscale
		self.myCam2.calibcorrect.updatePiDelay(1)
		self.myCam2.controllerView.setMinAmplitude(5)

		self.myCam.controllerView.startVideo()
		self.myCam2.controllerView.startVideo()
	

	    #######  data streaming 

		while self.playing:
			
			org1 = self.grayscale(1) #원본이미지파일 from each camera
			img1 = cv2.flip(org1,0) # 0은 이미지의 상하반전을 의미함. 
			gray1 = img1 

			org2 = self.grayscale(2)
			img2 = cv2.flip(org2,1) # 1은 이미지의 좌우반전을 의미함.
			gray2 = img2

			#stitching 
			detector = cv2.BRISK_create()
			keyPoints1, descriptors1 = detector.detectAndCompute(img1, None)
			keyPoints2, descriptors2 = detector.detectAndCompute(img2, None)
			#print('img1 - %d features, img2 - %d features' % (len(keyPoints1), len(keyPoints2)))
			#특징점 좌표 배열에 저장
			keyPoints1 = np.float32([keypoint.pt for keypoint in keyPoints1])
			keyPoints2 = np.float32([keypoint.pt for keypoint in keyPoints2])

			matches, H, status = matchKeypoints(keyPoints1, keyPoints2, descriptors1, descriptors2)
            # stitching success
			try:
				result = cv2.warpPerspective(img1, H,(img1.shape[1] + img2.shape[1], img2.shape[0]))
				result[0:img2.shape[0], 0:img2.shape[1]] = img2
				result=cv2.resize(result,(640,240))
				h, w  = result.shape

   			#if fail -> original non stitched images
			except:
				result = np.concatenate((img2, img1), axis = 1)
				result=cv2.resize(result,(640,240))

				h, w  = result.shape

			self.qimg = QImage(result.astype(np.int8), w, h, QImage.Format_Grayscale8)
			pixmap = QPixmap.fromImage(self.qimg)
			self._photo.setPixmap(pixmap)
			QApplication.processEvents()

			self.show()
