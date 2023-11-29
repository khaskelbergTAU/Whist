FROM gradle:8.4-jdk-alpine AS build
COPY --chown=gradle:gradle . /home/gradle/src
WORKDIR /home/gradle/src
RUN gradle buildFatJar --no-daemon

FROM amazoncorretto:20-alpine
RUN mkdir /app
RUN apk add build-base
EXPOSE 80:8080
COPY --from=build /home/gradle/src/build/libs/*.jar /app/server.jar
ENTRYPOINT ["java", "-jar", "/app/server.jar"]