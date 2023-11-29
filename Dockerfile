FROM gradle:8.4-jdk-alpine AS build
COPY --chown=gradle:gradle 'Whist-Server' /home/gradle/src
COPY --chown=gradle:gradpe API/whist.h /home/gradle/src/src/main/resources/static/pages/whist.h
WORKDIR /home/gradle/src
RUN ./gradlew buildFatJar --no-daemon

FROM amazoncorretto:20-alpine
RUN mkdir /app
RUN apk add build-base
EXPOSE 80:8080
COPY --from=build /home/gradle/src/build/libs/*.jar /app/server.jar
COPY API /app/
ENTRYPOINT ["java", "-jar", "/app/server.jar"]